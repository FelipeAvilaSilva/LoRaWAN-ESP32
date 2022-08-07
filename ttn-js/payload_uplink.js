function decodeUplink(input) {
    var data = {};
    var warnings = [];
    var errors = [];
    
    var latLng = function(bytes) {
  if (bytes.length !== latLng.BYTES) {
    throw new Error('Lat/Long must have exactly 8 bytes');
  }

  var lat = bytesToInt(bytes.slice(0, latLng.BYTES / 2));
  var lng = bytesToInt(bytes.slice(latLng.BYTES / 2, latLng.BYTES));

  return [lat / 1e6, lng / 1e6];
};
latLng.BYTES = 8;

var bytesToInt = function(bytes) {
  var i = 0;
  for (var x = 0; x < bytes.length; x++) {
    i |= +(bytes[x] << (x * 8));
  }
  return i;
};
	

    if(input.fPort == 2) {
      
      
        var temperature = (input.bytes[0]<<8) | input.bytes[1];
        data.temperature = temperature/100;
        var humidity = (input.bytes[2]<<8) | input.bytes[3];
        data.humidity = humidity/100;
        var voltage = (input.bytes[4]<<8) | input.bytes[5];
        data.voltage = voltage/100;
        var d_dht = input.bytes[6];
        data.d_dht = d_dht;
        var d_sd = input.bytes[7];
        data.d_sd = d_sd;
        var d_gps = input.bytes[17];
        data.d_gps = d_gps;
        /*var lat = Number(input.bytes[8]<< 8 | input.bytes[9] | 
        input.bytes[10]<< 8  | input.bytes[11]);
        data.lat = lat/100000;
        var lng = input.bytes[12]<< 8 | input.bytes[13] | 
        input.bytes[14]<< 8 | input.bytes[15];
        data.lng = lng/100000;*/
        /*var lat = input.bytes[8] + (input.bytes[9] << 8) + (input.bytes[10] << 16);
        data.lat = lat / 100000;
        var lng = input.bytes[11] + (input.bytes[12] << 8) + (input.bytes[13] << 16);
        data.lng = lng / 100000;*/
        var ll = latLng(input.bytes.slice(8, 16));
        data.lat = ll[0];
        data.lng = ll[1];
        
        
    } else {
        errors.push("payload unknown");
    }
    
    return {
        data: data,
        warnings: warnings,
        errors: errors
    };
} 