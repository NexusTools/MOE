(function(){
    var startTime = (new Date()).getTime();
    engine.debug("Using standard loader");
    var unresolvedDeps = [];
    var librariesToLoad = [];
    var loadedLibraries = [];

    var addLibraryDep = function(dep) {
        dep = Url(dep, "qrc:/libraries/").toString();
        if(dep.indexOf("qrc:/libraries/") == 0 && dep.indexOf(".") == -1)
            dep += ".js";

        if(loadedLibraries.indexOf(dep) != -1)
            return false;
        if(librariesToLoad.indexOf(dep) != -1)
            return false;
        if(unresolvedDeps.indexOf(dep) != -1)
            return true;

        librariesToLoad.push(dep);
        return true;
    }

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

    var loadContent = function(contentDesc) {
        engine.debug(contentDesc);
        engine.debug("Processing content package `"+contentDesc.Name['#text']+"`");
        var deps = [];
        if("Dependancies" in contentDesc)
            ;// TODO: Implement processing dependancies
        if(!deps.length)
            deps.push("standard");
        deps.forEach(addLibraryDep);
        loadNextLibrary();
    }

    engine.debug("Reading content metadata");
    var contentXML = ResourceRequest("info.xml");
    contentXML.error.connect(function(){loadContent({"Name":contentXML.url.toString()});})
    contentXML.receivedXML.connect(function(xmlData){
        try {
            engine.debug("Received XML Data: " + xmlData)
            loadContent(xmlData.MoeContent.ContentInfo);
        } catch(e) {
            try {
                throw "Failed to parse content package: " + contentXML.url + " corrupt.\n" + xmlData.errorMessage + " at Line " + xmlData.errorLine + " on Column" + xmlData.errorColumn;
            } catch(t) {
                throw "Failed to process content package: " + contentXML.url + " corrupt.\n" + e.toString();
            }
        }
    });

})();
