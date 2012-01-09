#This is a dummy implementation of GeoIPCity...

module GeoIPCity
  class Database
    def initialize dat_path, mode=:whatever, flag=false
      raise Errno::ENOENT unless File.exist? dat_path
      @dat_path = dat_path
    end
    def db
      @db ||= File.open('GeoLiteCity-Blocks.csv', 'r')
    end
    def look_up ip
      raise TypeError unless ip
#puts "/opt/GeoIP/bin/geoiplookup -f #{@dat_path} #{ip}"
      path = `which geoiplookup`
      result = `#{path} -f #{@dat_path} #{ip}`
      #raise result
      # GeoIP City Edition, Rev 1: US, NY, Syracuse, N/A, 43.051399, -76.149498, 555, 315
      country_code, region, city, lat, lng = result.scan(/: (.+?), (.+?), (.+?), .+?, (-?\d{1,3}.\d+), (-?\d{1,3}.\d+), \d+/).first
      
      names = {
        'US' => 'United States'
      }
      return {
        :city => city,
        :country_code => country_code,
        :country_name => names[country_code],
        :region => region,
        :latitude => lat.to_f,
        :longitude => lng.to_f
      }
    end
  end
end

404228096
404232216
404234239

123141
