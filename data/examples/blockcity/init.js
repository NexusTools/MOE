engine.setTicksPerSecond(30);
var surface = new GraphicsSurface("BlockCity", Size(800, 600));
var titleContainer = new GraphicsContainer(surface);
surface.background = Rgb(50, 0, 0);

function ready() {
    this.play = new GraphicsText("Play", Font("Ubuntu", 30), titleContainer);
    play.foreground = Rgba(255, 255, 255, 100);
}

surface.connected.connect(ready);
surface.resized.connect(function(size){
    engine.debug(size);
    titleContainer.setSize(surface.size());
});
titleContainer.setSize(surface.size());
//engine.tick.connect(func);
