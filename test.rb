require 'test/unit'
require File.dirname(__FILE__) + '/lib/geoip_city'
require 'rubygems'

class GeoIPTest < Test::Unit::TestCase

  def setup
    @dbfile = ENV['GEOIP_DB'] || 'GeoLiteCity.dat'
  end

  def test_construction_default
    db = GeoIPCity::Database.new(@dbfile)

    assert_raises TypeError do
      db.look_up(nil)
    end

    h = db.look_up('24.24.24.24')

    assert_kind_of Hash, h
    assert_equal 'Syracuse', h[:city]
    assert_equal 'United States', h[:country_name]
  end

  def test_construction_index
    db = GeoIPCity::Database.new(@dbfile, :index)
    h = db.look_up('24.24.24.24')
    assert_equal 'Syracuse', h[:city]
  end

  def test_construction_filesystem
    db = GeoIPCity::Database.new(@dbfile, :filesystem)
    h = db.look_up('24.24.24.24')
    assert_equal 'Syracuse', h[:city]
  end

  def test_construction_memory
    db = GeoIPCity::Database.new(@dbfile, :memory)
    h = db.look_up('24.24.24.24')
    assert_equal 'Syracuse', h[:city]
  end

  def test_construction_filesystem_check
    db = GeoIPCity::Database.new(@dbfile, :filesystem, true)
    h = db.look_up('24.24.24.24')
    assert_equal 'Syracuse', h[:city]
  end

  def test_bad_db_file
    assert_raises Errno::ENOENT do
      GeoIPCity::Database.new('/blah')
    end
  end

  if defined? Encoding
    def test_string_encoding_support
      Encoding.default_internal = nil
      db = GeoIPCity::Database.new(@dbfile, :index)
      h = db.look_up('24.24.24.24')
      assert_equal Encoding.find('utf-8'), h[:city].encoding

      Encoding.default_internal = Encoding.find('us-ascii')
      db2 = GeoIPCity::Database.new(@dbfile, :index)
      h2 = db2.look_up('24.24.24.24')
      assert_equal Encoding.find('us-ascii'), h2[:city].encoding
    end
  end

end
