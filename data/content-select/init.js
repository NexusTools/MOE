
engine.debug(args.keys());

var i = 0;
engine.setTicksPerSecond(60);
engine.tick.connect(engine, function() {
    i++;
    if(i >= 60)
        engine.quit();
    engine.debug("Tuna Fish: " + i);
});

if(!("surface" in this))
    engine.abort("This program requires a graphical surface.");
