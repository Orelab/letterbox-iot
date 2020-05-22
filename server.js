const express = require('express')
const app = express()

app.get('/', function(req, res) {
    res.send('nothing')
})

app.get('/time', function (req, res) {
    res.send( "" + Date.now() )
})

app.get('/:message', function(req, res) {
    var msg = req.params.message;

    console.log(msg)
    res.send('ok')
})

app.listen(3000, function () {
    console.log('App listening on port 3000!')
})

