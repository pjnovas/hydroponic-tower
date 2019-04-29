var mosca = require('mosca');

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

var server = new mosca.Server(settings);

server.on('clientConnected', function(client) {
  console.log('client connected', client.id);
});

server.on('published', function(packet, client) {
  console.log(decoder.end(packet.payload));
});

server.on('ready', setup);

function setup() {
  console.log('Mosca server is up and running');
}
