var titleMenuButtonForgroundColor = Rgba(255, 255, 255, 200);
var titleMenuButtonForgroundColorHovered = Rgba(255, 255, 255, 255);
var titleMenuButtonForgroundColorActive = Rgba(80, 0, 0, 255);
var titleMenuFont = Font("monospace", 26);

engine.setTicksPerSecond(30);
var surface = new GraphicsSurface("BlockCity", Size(800, 600));
surface.background = Rgb(50, 0, 0);

var curScreen = null;
var curLevel = new Level();

function TitleMenu() {
    this.gC = new GraphicsContainer(surface);
    var playGameButton = new GraphicsText("Play!", titleMenuFont, this.gC);
    playGameButton.foreground = titleMenuButtonForgroundColor;
    playGameButton.mouseEntered.connect(function() {
        playGameButton.animate("foreground", titleMenuButtonForgroundColorHovered);
    });

    playGameButton.mouseLeft.connect(function() {
        playGameButton.animate("foreground", titleMenuButtonForgroundColor);
    });

    playGameButton.mousePressed.connect(function() {
        playGameButton.killAnimation("foreground");
        playGameButton.foreground = titleMenuButtonForgroundColorActive;
        switchSurfaceContents(1);
    });


    var levelEditorButton = new GraphicsText("Level Editor", titleMenuFont, this.gC);
    levelEditorButton.foreground = titleMenuButtonForgroundColor;
    levelEditorButton.mouseEntered.connect(function() {
        levelEditorButton.animate("foreground", titleMenuButtonForgroundColorHovered);
    });

    levelEditorButton.mouseLeft.connect(function() {
        levelEditorButton.animate("foreground", titleMenuButtonForgroundColor);
    });

    levelEditorButton.mousePressed.connect(function() {
        levelEditorButton.killAnimation("foreground");
        levelEditorButton.foreground = titleMenuButtonForgroundColorActive;
        switchSurfaceContents(2);
    });


    var exitGameButton = new GraphicsText("Exit", titleMenuFont, this.gC);
    exitGameButton.foreground = titleMenuButtonForgroundColor;
    exitGameButton.mouseEntered.connect(function() {
        exitGameButton.animate("foreground", titleMenuButtonForgroundColorHovered);
    });

    exitGameButton.mouseLeft.connect(function() {
        exitGameButton.animate("foreground", titleMenuButtonForgroundColor);
    });

    exitGameButton.mousePressed.connect(function() {
        exitGameButton.killAnimation("foreground");
        exitGameButton.foreground = titleMenuButtonForgroundColorActive;
        engine.quit();
    });


    this.handleResize = function(size) {
        playGameButton.setPos(size.width / 2 - playGameButton.width / 2, (size.height / 2 - playGameButton.height / 2) - 64);
        levelEditorButton.setPos(size.width / 2 - levelEditorButton.width / 2, (size.height / 2 - levelEditorButton.height / 2));
        exitGameButton.setPos(size.width / 2 - exitGameButton.width / 2, (size.height / 2 - exitGameButton.height / 2) + 64);
        this.gC.setSize(size);
    }
}

function InGameMenu() {
    this.gC = new GraphicsContainer(surface);
    var gOC = this.gC;

    gOC.mouseMoved.connect(function(point) {
        { //Stub block for if the space bar is down.
            curLevel.entities[curLevel.entities.length] = new Entity(point.x, point.y, 50, 75, "white");
        }
    });

    this.handleResize = function(size) {
        this.gC.setSize(size);
    }
}

function Level() {
    this.entities = [];
}

function Entity(x, y, width, height, col) {
    engine.debug("x: " + x + ", y: " + y + ", w: " + width + ", h: " + height + ", col: " + col);
    this.gO = new GraphicsObject(surface);
    this.gO.background = col;
    this.gO.setPos(x, y);
    this.gO.setSize(width, height);
}

function switchSurfaceContents(int) {
    if(curScreen == null)
        int=0;
    else {
        engine.debug(curScreen);
        surface.remove(curScreen.gC);
        curScreen = null;
    }
    switch(int) {
    case 0:
        curScreen = new TitleMenu();
        break;
    case 1:
        curScreen = new InGameMenu();
        break;
    case 2:
        break;
    }

    surface.resized.connect(function(size){
        curScreen.handleResize(size);
    });
    curScreen.gC.setSize(surface.size());
}
switchSurfaceContents(0);
//surface.connected.connect(switchSurfaceContents);

//engine.tick.connect(func);
