
var surface = new GraphicsSurface("Select Content (MOE Game Engine v" + engine.version + ")", Size(800, 600));
surface.background = "dark magenta";

var title = new GraphicsText("Select Content", Font("Arial", 16), surface);
title.foreground = "white";

surface.resized.connect(function(size){
    title.x = size.width/2 - title.width/2;
});

setTimeout(function(){
    surface.setSize(300, 300);
}, 2000);
