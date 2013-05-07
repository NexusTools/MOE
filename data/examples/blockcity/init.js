var titleMenuButtonForgroundColor = Rgba(255, 255, 255, 200);
var titleMenuButtonForgroundColorHovered = Rgba(255, 255, 255, 255);
var titleMenuButtonForgroundColorActive = Rgba(80, 0, 0, 255);
var titleMenuFont = Font("monospace", 26);

engine.setTicksPerSecond(30);
var surface = new GraphicsSurface("BlockCity", Size(800, 600));
var titleContainer = new GraphicsContainer(surface);
surface.background = Rgb(50, 0, 0);

var curScreen = null;

function titleMenuReady() {
    this.playGameButton = new GraphicsText("Play!", titleMenuFont, titleContainer);
    playGameButton.foreground = titleMenuButtonForgroundColor;
    playGameButton.mouseEntered.connect(function() {
        playGameButton.animate("foreground", titleMenuButtonForgroundColorHovered);
    });

    playGameButton.mouseLeft.connect(function() {
        playGameButton.animate("foreground", titleMenuButtonForgroundColor);
    });

    playGameButton.mousePressed.connect(function(/*point, buttonType*/) {
        playGameButton.killAnimation("foreground");
        playGameButton.foreground = titleMenuButtonForgroundColorActive;
    });


    this.levelEditorButton = new GraphicsText("Level Editor", titleMenuFont, titleContainer);
    levelEditorButton.foreground = titleMenuButtonForgroundColor;
    levelEditorButton.mouseEntered.connect(function() {
        levelEditorButton.animate("foreground", titleMenuButtonForgroundColorHovered);
    });

    levelEditorButton.mouseLeft.connect(function() {
        levelEditorButton.animate("foreground", titleMenuButtonForgroundColor);
    });

    levelEditorButton.mousePressed.connect(function(/*point, buttonType*/) {
        levelEditorButton.killAnimation("foreground");
        levelEditorButton.foreground = titleMenuButtonForgroundColorActive;
    });


    this.exitGameButton = new GraphicsText("Exit", titleMenuFont, titleContainer);
    exitGameButton.foreground = titleMenuButtonForgroundColor;
    exitGameButton.mouseEntered.connect(function() {
        exitGameButton.animate("foreground", titleMenuButtonForgroundColorHovered);
    });

    exitGameButton.mouseLeft.connect(function() {
        exitGameButton.animate("foreground", titleMenuButtonForgroundColor);
    });

    exitGameButton.mousePressed.connect(function(/*point, buttonType*/) {
        exitGameButton.killAnimation("foreground");
        exitGameButton.foreground = titleMenuButtonForgroundColorActive;
    });


    this.resizeTitleScreen = function(size) {
        playGameButton.setPos(size.width / 2 - playGameButton.width / 2, (size.height / 2 - playGameButton.height / 2) - 64);
        levelEditorButton.setPos(size.width / 2 - levelEditorButton.width / 2, (size.height / 2 - levelEditorButton.height / 2));
        exitGameButton.setPos(size.width / 2 - exitGameButton.width / 2, (size.height / 2 - exitGameButton.height / 2) + 64);
    }
    curScreen = titleContainer;
}

surface.connected.connect(titleMenuReady);
surface.resized.connect(function(size){
    engine.debug(size);
    titleContainer.setSize(surface.size());
    if(curScreen == titleContainer) {
        resizeTitleScreen(size);
    }
});
titleContainer.setSize(surface.size());
//engine.tick.connect(func);
