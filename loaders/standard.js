(function(){
    engine.debug("Using standard loader");
    var libraries = ["prototype", "shared"];

    var evalGlobal = function(source, filename) {
        engine.debug("Evaluating `" + filename + "`, " + source.length + " bytes");
        eval.apply(global, [source, filename]);
    }

    var loadInitFile = function() {
        var initFile = new ResourceRequest("init.js");
        var processInitFile = (function(source){
            initFile.receivedString.disconnect(processInitFile);
            evalGlobal(source, "init");
        }).bind(this);
        initFile.receivedString.connect(processInitFile);
        initFile.error.connect(function(error){
            throw "Error downloading init file: " + error;
        });
    }

    var loadNextLibrary = function() {
        if(!libraries.length) {
            loadInitFile();
            return;
        }

        var lib = libraries.shift();
        var libFile = new ResourceRequest(Url(lib + ".js", "qrc:/libraries/"));
        libFile.receivedString.connect(function(source){
            evalGlobal(source, lib);
            loadNextLibrary();
        });
        libFile.error.connect(function(error){
            throw "Error downloading library `" + lib + "`: " + error;
        });
    }
    loadNextLibrary();
})();
