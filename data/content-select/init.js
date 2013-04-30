
if(!("surface" in this))
    engine.abort("This program requires a graphical surface.");

/*
var boardList = new ResourceRequest("https://api.4chan.org/boards.json");
boardList.error.connect(function(boardData) {
    engine.debug(boardData);
});
boardList.complete.connect(function(boardData) {
    engine.debug(boardData);
});
engine.debug(boardList);

engine.tick.connect(engine, function() {
});
*/

surface.paint.connect(surface, function(p){
    engine.debug(p);
    engine.debug("Paint Event");
    engine.debug(surface.width());
    var color = ["grey","purple","red"];
    var x = 0;
    var y = 0;
    p.fillRect(Rect(0, 0, surface.width(), surface.height()), 0);
    for(var i in color) {
        p.fillRect(Rect(x, y, 50, 50), color[i]);
        y+=50;
    }
});
