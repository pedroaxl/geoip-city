/* ry dahl <ry@tinyclouds.org> May 21, 2007 */
/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */
#include <ruby.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

#ifdef HAVE_RUBY_ENCODING_H
#include <ruby/encoding.h>
static rb_encoding *utf8Encoding;
#endif

static VALUE cDB;
static VALUE rb_geoip_memory;
static VALUE rb_geoip_filesystem;
static VALUE rb_geoip_index;

#ifdef HAVE_RUBY_ENCODING_H
#define ASSOCIATE_ENCODING(str) \
  rb_enc_associate(str, utf8Encoding);  \
  if (default_internal_enc) { \
    str = rb_str_export_to_enc(str, default_internal_enc); \
  }
#else
#define ASSOCIATE_ENCODING(str)
#endif

/* The first argument is the filename of the GeoIPCity.dat file
 * load_option = :standard, :index, or :memory. default :memory
 * check_cache = true or false. default false
 *
 * filesystem: read database from filesystem, uses least memory.
 *
 * index: the most frequently accessed index portion of the database,
 *   resulting in faster lookups than :filesystem, but less memory usage than
 *   :memory.
 *
 * memory: load database into memory, faster performance but uses more
 *   memory.
 */
static VALUE rb_geoip_new(int argc, VALUE *argv, VALUE self)
{
  GeoIP *gi;
  VALUE database = Qnil;
  VALUE filename, load_option = Qnil, check_cache = Qnil;
  int flag;

  rb_scan_args(argc, argv, "12", &filename, &load_option, &check_cache);
  if(NIL_P(load_option))
    load_option = rb_geoip_memory;
  if(NIL_P(check_cache))
    check_cache = Qfalse;
  Check_Type(load_option, T_SYMBOL);

  if(load_option == rb_geoip_memory) {
    flag = GEOIP_MEMORY_CACHE;
  } else if(load_option == rb_geoip_filesystem) {
    flag = GEOIP_STANDARD;
  } else if(load_option == rb_geoip_index) {
    flag = GEOIP_INDEX_CACHE;
  } else {
    rb_raise(rb_eTypeError, "the second option must be :memory, :filesystem, or :index");
    return Qnil;
  }

  if(RTEST(check_cache)) flag |= GEOIP_CHECK_CACHE;

  if(gi = GeoIP_open(StringValuePtr(filename), flag)) {
    database = Data_Wrap_Struct(cDB, 0, GeoIP_delete, gi);
    rb_obj_call_init(database, 0, 0);
  } else {
    rb_sys_fail("Problem opening database");
  }
  return database;
}

/* helper  */
void rb_hash_sset(VALUE hash, const char *str, VALUE v) {
  rb_hash_aset(hash, ID2SYM(rb_intern(str)), v);
}

VALUE rb_record_to_hash (GeoIPRecord *record)
{
  VALUE hash = rb_hash_new();
#ifdef HAVE_RUBY_ENCODING_H
  rb_encoding *default_internal_enc = rb_default_internal_encoding();
#endif

  if(record->country_code) {
    VALUE str = rb_str_new2(record->country_code);
    ASSOCIATE_ENCODING(str);
    rb_hash_sset(hash, "country_code", str);
  }

  if(record->country_code3) {
    VALUE str = rb_str_new2(record->country_code3);
    ASSOCIATE_ENCODING(str);
    rb_hash_sset(hash, "country_code3", str);
  }

  if(record->country_name) {
    VALUE str = rb_str_new2(record->country_name);
    ASSOCIATE_ENCODING(str);
    rb_hash_sset(hash, "country_name", str);
  }

  if(record->region) {
    VALUE str = rb_str_new2(record->region);
    ASSOCIATE_ENCODING(str);
    rb_hash_sset(hash, "region", str);
  }

  if(record->city) {
    VALUE str = rb_str_new2(record->city);
    ASSOCIATE_ENCODING(str);
    rb_hash_sset(hash, "city", str);
  }

  if(record->postal_code)  {
    VALUE str = rb_str_new2(record->city);
    ASSOCIATE_ENCODING(str);
    rb_hash_sset(hash, "postal_code", rb_str_new2(record->postal_code));
  }

  if(record->latitude)
    rb_hash_sset(hash, "latitude", rb_float_new((double)record->latitude));
  if(record->longitude)
    rb_hash_sset(hash, "longitude", rb_float_new((double)record->longitude));
  if(record->dma_code)
    rb_hash_sset(hash, "dma_code", INT2NUM(record->dma_code));
  if(record->area_code)
    rb_hash_sset(hash, "area_code", INT2NUM(record->area_code));

  return hash;
}

/* Pass this function an IP address as a string, it will return a hash
 * containing all the information that the database knows about the IP
 *    db.look_up('24.24.24.24')
 *    => {:city=>"Ithaca", :latitude=>42.4277992248535,
 *        :country_code=>"US", :longitude=>-76.4981994628906,
 *        :country_code3=>"USA", :dma_code=>555,
 *        :country_name=>"United States", :area_code=>607,
 *        :region=>"NY"}
 */
VALUE rb_geoip_look_up(VALUE self, VALUE addr) {
  GeoIP *gi;
  GeoIPRecord *record = NULL;
  VALUE hash = Qnil;

  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  if(record = GeoIP_record_by_addr(gi, StringValuePtr(addr))) {
    hash =  rb_record_to_hash(record);
    GeoIPRecord_delete(record);
  }
  return hash;
}

void Init_geoip_city ()
{
  VALUE mGeoIP = rb_define_module("GeoIPCity");

  rb_geoip_memory = ID2SYM(rb_intern("memory"));
  rb_geoip_filesystem = ID2SYM(rb_intern("filesystem"));
  rb_geoip_index = ID2SYM(rb_intern("index"));

  cDB = rb_define_class_under(mGeoIP, "Database", rb_cObject);
  rb_define_singleton_method(cDB, "new", rb_geoip_new, -1);
  rb_define_method(cDB, "look_up", rb_geoip_look_up, 1);

#ifdef HAVE_RUBY_ENCODING_H
  utf8Encoding = rb_utf8_encoding();
#endif
}
