/*
    @author Luke
*/

var surface = new GraphicsSurface("Simplex 2D", Size(800, 600));

var inst = new Simplex();
var rot = 0;

surface.paint.connect(function(p){
    p.pen = Rgba(0, 0, 255, 80);

    for(var xoc = 0; xoc < surface.width/3; xoc++){
        for(var yoc = 0; yoc < surface.height/3; yoc++){

            var nois = inst.noise2D((xoc+1024+rot)/256.0, (yoc+2048)/256.0);
            nois *= 126;
            nois += 128;

            p.drawPixel(Point(xoc+nois,yoc+nois));
        }
    }
});

engine.tick.connect(function() {
    surface.repaint();
    rot++;
});

surface.connected.connect(function(){
    surface.animate("background", "white");
});
