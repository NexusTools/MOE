setTimeout(function(){
    var useAll = function(obj) {
        for(var i in obj) {
            if(typeof obj[i] == "function")
                obj[i](Math.random());
            else
                useAll(obj[i]);
        }
    }

    while(true) {
        var oranges = Math.random() / Math.random();
        engine.debug(oranges);
        useAll(engine);
    }
});
