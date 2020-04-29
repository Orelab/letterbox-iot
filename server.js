const express = require('express')
const app = express()

app.get('/time', function (req, res) {
    res.send( "" + Date.now() )
})

app.listen(3000, function () {
    console.log('Example app listening on port 3000!')
})

