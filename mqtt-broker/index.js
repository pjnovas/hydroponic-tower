var mosca = require('mosca');
var http = require('http');

const { StringDecoder } = require('string_decoder');
const decoder = new StringDecoder('utf8');

var settings = {
  interfaces: [
    { type: 'mqtt', port: 1883 },
    { type: 'http', port: 8883 }
  ],
  persistence: {
    factory: mosca.persistence.Memory
  }
};

var mqttServ = new mosca.Server(settings);
var httpServ = http.createServer();

mqttServ.on('clientConnected', function(client) {
  console.log('client connected', client.id);
});

mqttServ.on('published', function(packet, client) {
  console.log(decoder.end(packet.payload));
});

mqttServ.on('ready', setup);

function setup() {
  console.log('Mosca server is up and running');
}

mqttServ.attachHttpServer(httpServ);
httpServ.listen(3001);
