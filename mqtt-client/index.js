var mqtt = require('mqtt')
var client  = mqtt.connect('mqtt://localhost')

client.on('connect', function () {
  console.log('connected!');

  client.subscribe('outTopic', function (err) {
    if (!err) {
      client.publish('inTopic', 'Hello mqtt from nodejs client')
    }
  })
})

client.on('message', function (topic, message) {
  // message is Buffer
  console.log([topic, message.toString()].join(": "))
  // client.end()
})