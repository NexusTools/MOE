
var surface = new GraphicsSurface("Select Content (MOE Game Engine v" + engine.version + ")", Size(800, 600));

var title = new GraphicsText("Select Content", Font("Arial", 16), surface);
title.foreground = "white";

var snakes = [];

surface.keyPressed.connect(function(c){
    engine.debug(c);
});

function spawnSnakes() {
    for(var i=0; i<15; i++) {
        var pos = Point(Math.random()*surface.width, Math.random()*surface.height);
        var trail = [];
        for(var t=0; t<10; t++)
            trail.push(Point(pos.x, pos.y));
        snakes.push({
            "trail": trail,
            "speed": Point(-8+Math.random()*16, -8+Math.random()*16)
                    });
    }
}

engine.tick.connect(function(){
    var center = Point(surface.width/2,surface.height/2);
    for(var i=0; i<snakes.size(); i++) {
        var lastTrail = snakes[i].trail.length-1;
        for(var t=0; t<lastTrail; t++) {
            surface.repaint(Rect(snakes[i].trail[t].x-5,snakes[i].trail[t].y-5,11,11));
            snakes[i].trail[t] = snakes[i].trail[t+1];
        }
        surface.repaint(Rect(snakes[i].trail[t].x-5,snakes[i].trail[t].y-5,11,11));

        var cSpeed = snakes[i].speed;
        var cPos = snakes[i].trail[t];
        cSpeed.x += (center.x - cPos.x) / 14;
        cSpeed.y += (center.y - cPos.y) / 14;
        cSpeed.x += -2 + Math.random() * 4;
        cSpeed.y += -2 + Math.random() * 4;
        cSpeed.x = Math.clamp(-(surface.width/14), cSpeed.x, surface.width/14);
        cSpeed.y = Math.clamp(-(surface.height/14), cSpeed.y, surface.height/14);
        cPos.x += cSpeed.x / 7;
        cPos.y += cSpeed.y / 7;
        snakes[i].speed = cSpeed;
        snakes[i].trail[t] = Point(cPos.x, cPos.y);
        surface.repaint(Rect(snakes[i].trail[t].x-5,snakes[i].trail[t].y-5,11,11));
    }
});

surface.foreground = "light blue";
var snakePenColor = surface.foreground;
snakePenColor.alpha = 60;
surface.foreground = snakePenColor;
surface.paint.connect(function(p) {
    for(var h=1; h<6; h+=2){
        p.setPenThickness(h);
        for(var i=0; i<snakes.length; i++) {
            var lastPos=false;
            for(var t=0; t<snakes[i].trail.length; t++) {
                if(lastPos)
                    p.drawLine(lastPos,snakes[i].trail[t]);
                lastPos = snakes[i].trail[t];
            }
        }
    }
});

surface.resized.connect(function(size){
    title.x = size.width/2 - title.width/2;
});

function start(size) {
    if(size.width < 1 || size.height < 1)
        return;
    surface.resized.disconnect(start);
    surface.animate("background", "black");
    spawnSnakes();
}

surface.resized.connect(start);

engine.debug(surface.background);
