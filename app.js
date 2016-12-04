var express = require('express');
var bodyParser = require('body-parser');
var mongodb = require('mongodb');

var app = express();
var tasks;

app.use(express.static('front'));
app.use(bodyParser.json());
app.listen(3000);

mongodb.MongoClient.connect("mongodb://localhost:27017/test", function(err, database) {
  tasks = database.collection("tasks");
});


app.get(   "/api/tasks", function(req, res) {
  tasks.find().toArray (function(err, items) {
   res.send(items);
  });
}  );


app.get("/api/task/:_id", function(req, res) {
  users.findOne({_id: mongodb.ObjectID(req.params._id)}, function(err, item) {
   res.send(item);
  });
});


app.post("/api/tasks", function(req, res) {
  var task = req.body;
  if (task._id) task._id = mongodb.ObjectID(task._id);
  tasks.save(task, function() {
   res.send("sounyu ka hensyuu");
  });
});


app.delete("/api/tasks/:_id", function(req, res) {
  tasks.remove({_id: mongodb.ObjectID(req.params._id)}, function() {
   res.send("kesita");
  });
});
