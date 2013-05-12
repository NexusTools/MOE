(function(){
    var startTime = (new Date()).getTime();
    engine.debug("Using standard loader");
    engine.debug(Url(".").toString());
    var unresolvedDeps = [];
    var librariesToLoad = [];
    var loadedLibraries = [];

    var addLibraryDep = function(dep) {
        engine.debug(dep);
        dep = Url(dep, "qrc:/libraries/").toString();
        if(dep.indexOf("qrc:/libraries/") == 0 && dep.indexOf(".") == -1)
            dep += ".js";
        engine.debug(dep);

        if(loadedLibraries.indexOf(dep) != -1)
            return false;
        if(librariesToLoad.indexOf(dep) != -1)
            return false;
        if(unresolvedDeps.indexOf(dep) != -1)
            return true;

        librariesToLoad.push(dep);
        return true;
    }

     // TODO: Parse content XML file if exists
    engine.debug("Using standard libraries");
    addLibraryDep("standard");

    var evalGlobal = function(source, filename) {
        engine.debug("Evaluating `" + filename + "`, " + source.length + " bytes");
        eval.apply(global, [source, filename]);
    }

    var bind = function(ctx, func) {
        return function() {
            return func.apply(ctx, arguments);
        }
    }

    var loadInitFile = function() {
        engine.debug(loadedLibraries.length + " libraries loaded in " + ((new Date()).getTime() - startTime) + "ms");
        engine.debug(loadedLibraries);

        engine.debug("Loading init file");
        var initUrl = Url("init.js");
        var initFile = new ResourceRequest(initUrl);
        var processInitFile = function(source){
            evalGlobal(source, initUrl);
        };
        initFile.receivedString.connect(bind(this,processInitFile));
        initFile.error.connect(function(error){
            throw "Error downloading init file: " + error;
        });
    }

    var loadNextLibrary = function() {
        if(!librariesToLoad.length) {
            if(!unresolvedDeps.length) {
                loadInitFile();
                return;
            }
            var nextSet = unresolvedDeps;
            unresolvedDeps = [];
            nextSet.forEach(addLibraryDep);
        }

        var lib = librariesToLoad.shift();
        engine.debug("Loading library `" + lib + "`");
        var libFile = new ResourceRequest(lib);
        libFile.receivedString.connect(function(source){
            if(source.indexOf("/*") == 0) {
                try {
                    var metaString = source.substring(2, source.indexOf("*/"));
                    var metaObject = {};
                    var needsDep = false;
                    metaString.split("\n").forEach(function(line){
                        line = line.trim();
                        if(line.indexOf("@") == 0) {
                            var key = line.substring(1);
                            var pos = key.indexOf(" ");
                            var val = key.substring(pos+1);
                            key = key.substring(0, pos);
                            pos = val.indexOf(" ");
                            while(pos == 0) {
                                val = val.substring(1);
                                pos = val.indexOf(" ");
                            }
                            val = eval("(" + val + ")");
                            if(key == "dependancies") {
                                val.forEach(function(dep){
                                    if(addLibraryDep(dep))
                                        needsDep = true;
                                });
                            }
                        }
                    });

                    if(needsDep) {
                        unresolvedDeps.push(lib);
                        engine.setTimeout(loadNextLibrary);
                        return;
                    }
                } catch(e) {
                    throw "Error parsing metadata of library `" + lib + "`.\n" + e.toString();
                }
            }

            try {
                evalGlobal(source, lib);
            } catch(e) {
                throw "Error loading library `" + lib + "`.\n" + e.toString();
            }
            loadedLibraries.push(lib);
            engine.setTimeout(loadNextLibrary);
        });
        libFile.error.connect(function(error){
            throw "Error downloading library `" + lib + "`: " + error;
        });
    }
    loadNextLibrary();
})();
