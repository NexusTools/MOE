(function(){
    var mainFile = new ResourceRequest("./");
    mainFile.receivedString.connect(function(source){
        eval.apply(global, source);
    });
    mainFile.error.connect(function(error){
        throw "Error downloading main file: " + error;
    });
})();
