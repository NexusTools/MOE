var PLAYER = 0, WALL = 1, EASY_ENEMY = 2;

var titleMenuButtonForgroundColor = Rgba(255, 255, 255, 200);
var titleMenuButtonForgroundColorHovered = Rgba(255, 255, 255, 255);
var titleMenuButtonForgroundColorActive = Rgba(80, 0, 0, 255);
var titleMenuFont = Font("monospace", 26);

engine.setTicksPerSecond(30);
var surface = new GraphicsSurface("BlockCity", Size(800, 600));
surface.background = Rgb(50, 0, 0);

var curScreen = null;
var curLevel = null;

var player = null;

function tick() {
    if(curLevel == null)
        return;
    for(var i = 0; i < curLevel.tickable.length; i++) {
        curLevel.tickable[i].process();
    }
}

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

    gOC.mousePressed.connect(function(point) {
        { //Stub block for if the space bar is down.
            curLevel.tickable[curLevel.tickable.length] = new Entity(point.x, point.y, 50, 75, "white", EASY_ENEMY);
        }
    });

    this.handleResize = function(size) {
        this.gC.setSize(size);
        curLevel.dummy[0].gO.setSize(25, size.height);
        curLevel.dummy[1].gO.setPos(size.width - 25, 0);
        curLevel.dummy[1].gO.setSize(25, size.height);
        curLevel.dummy[2].gO.setPos(0, size.height - 25);
        curLevel.dummy[2].gO.setSize(size.width, 25);
    }
}

function Level() {
    this.tickable = [];
    this.dummy = [];
    this.dummy[0] = new Entity(0, 0, 25, surface.height, "grey", WALL);
    this.dummy[1] = new Entity(surface.width - 25, 0, 25, surface.height, "grey", WALL);
    this.dummy[2] = new Entity(0, surface.height - 25, surface.width, 25, "grey", WALL);
    player = new Entity(0, 0, 50, 75, PLAYER);
}

function Entity(x, y, width, height, col, type) {
    //engine.debug("New Entity: x: " + x + ", y: " + y + ", w: " + width + ", h: " + height + ", col: " + col);
    this.gO = new GraphicsObject(curScreen.gC);
    this.gO.background = col;
    this.gO.setPos(x, y);
    this.gO.setSize(width, height);
    this.onGround = true;
    this.harmful = false;
    this.xVel = 0;
    this.yVel = 0;
    switch(type) {
    case EASY_ENEMY:
        this.process = function() {
            processPhysics(this, true, true, true);
            this.harmful = true;
        }
        break;
    }
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
        curLevel = new Level();
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

engine.tick.connect(tick);

function processPhysics(ent, doUntickable, doTickable, doPly) {
        var rtn;
        var collideable2 = [];
        collideable2 = collideable2.concat((doUntickable ? curLevel.dummy : []), (doTickable ? curLevel.tickable : []), (doPly ? player : []));

        /* Compiles a list of actual valid entities within the area of the entity that's being processed expanded by a radius of the entities size. */
        var collideable = [];
        var i2 = 0;
        var nx = ent.gO.posX + ent.xVel;
        var ny = ent.gO.posY + ent.yVel;
        var nw = ent.gO.width * 2;
        var nh = ent.gO.height * 2;
        for(var i = 0; i < collideable2.length; i++) {
                if(collideable2[i] == ent)
                        continue;
                if(nx + nw >= collideable2[i].gO.posX && nx - (nw / 2) <= collideable2[i].gO.posX + collideable2[i].gO.width && ny + nh >= collideable2[i].gO.posY && ny - (nh / 2) <= collideable2[i].gO.posY + collideable2[i].gO.height)
                        collideable[i2++] = collideable2[i];
        }
        delete nx;
        delete ny;
        delete nw;
        delete nh;
        delete i2;
        delete collideable2;

        /* Actual AABB physics - I've made this for my old BlockCity project for a old version of Moe, converted to the new engine. */
        if(!ent.onGround)
                if(ent.yVel < ent.gO.height)
                        ent.yVel += 1.5;

        if(Math.abs(ent.yVel) > 0)
                ent.onGround = false;

        if(ent.xVel > 0) {
                ent.xVel--;
                if(ent.xVel < 0)
                        ent.xVel = 0;
        } else if(ent.xVel < 0) {
                ent.xVel++;
                if(ent.xVel > 0)
                        ent.xVel = 0;
        }

        if(ent.yVel > 0) {
                ent.yVel--;
                if(ent.yVel < 0)
                        ent.yVel = 0;
        } else if(ent.yVel < 0) {
                ent.yVel++;
                if(ent.yVel > 0)
                        ent.yVel = 0;
        }

        if(ent.gO.posX + ent.xVel < 0) {
                ent.gO.posX = 0;
                ent.xVel = 0;
                rtn = 1;
        } else if(ent.gO.posX + ent.gO.width + ent.xVel > surface.width) {
                ent.gO.posX = surface.width - ent.gO.width;
                ent.xVel = 0;
        } else {
                if(ent.xVel != 0) {
                        for(var i = 0; i < collideable.length; i++) {
                                if(ent.xVel > 0) {
                                        if(ent.gO.posX + ent.gO.width + ent.xVel > collideable[i].gO.posX && ent.gO.posX + ent.xVel < collideable[i].gO.posX && ent.gO.posY + ent.gO.height > collideable[i].gO.posY && ent.gO.posY < collideable[i].gO.posY + collideable[i].gO.height) {
                                                ent.gO.posX = collideable[i].gO.posX - ent.gO.width;
                                                if((collideable[i] == player && (ent.harmful)) || ((collideable[i].harmful) && ent == player)) {
                                                        player.xVel = 10 + Math.random() * 5;
                                                        player.yVel = -(10 + Math.random() * 5);
                                                        player.takeHealth(10);
                                                }
                                                ent.xVel = 0;
                                                rtn = (ent == player ? collideable[i] : 2);
                                        }
                                } else if(ent.xVel < 0) {
                                        if(ent.gO.posX + ent.gO.width + ent.xVel > collideable[i].gO.posX + collideable[i].gO.width && ent.gO.posX + ent.xVel < collideable[i].gO.posX + collideable[i].gO.width && ent.gO.posY + ent.gO.height > collideable[i].gO.posY && ent.gO.posY < collideable[i].gO.posY + collideable[i].gO.height) {
                                                ent.gO.posX = collideable[i].gO.posX + collideable[i].gO.width;
                                                if((collideable[i] == player && (ent.harmful)) || ((collideable[i].harmful) && ent == player)) {
                                                        player.xVel = -(10 + Math.random() * 5);
                                                        player.yVel = -(10 + Math.random() * 5);
                                                        player.takeHealth(10);
                                                }
                                                ent.xVel = 0;
                                                rtn = (ent == player ? collideable[i] : 1);
                                        }
                                }
                        }
                }
        }

        if(ent.gO.posY + ent.yVel < 0) {
                ent.gO.posY = 0;
                ent.yVel = 0;
        } else if(ent.gO.posY + ent.gO.height + ent.yVel > surface.height) {
                ent.gO.posY = surface.height - ent.gO.height;
                ent.yVel = 0;
                ent.onGround = true;
        } else {
                if(ent.yVel != 0) {
                        for(i = 0; i < collideable.length; i++) {
                                if(ent.yVel < 0) {
                                        if(ent.gO.posY + ent.yVel + ent.gO.height > collideable[i].gO.posY + collideable[i].gO.height && ent.gO.posY + ent.yVel < collideable[i].gO.posY + collideable[i].gO.height && ent.gO.posX + ent.gO.width > collideable[i].gO.posX && ent.gO.posX < collideable[i].gO.posX + collideable[i].gO.width) {
                                                ent.gO.posY = collideable[i].gO.posY + collideable[i].gO.height;
                                               if((collideable[i] == player && (ent.harmful)) || ((collideable[i].harmful) && ent == player)) {
                                                        player.xVel = (Math.round(Math.random() * 1) == 1 ? -(10 + Math.random() * 5) : 10 + Math.random() * 5);
                                                        player.takeHealth(10);
                                                } else
                                                        ent.yVel = 0;
                                                rtn = collideable[i];
                                        }
                                } else if(ent.yVel > 0) {
                                        if(ent.gO.posY + ent.yVel + ent.gO.height > collideable[i].gO.posY && ent.gO.posY + ent.yVel < collideable[i].gO.posY && ent.gO.posX + ent.gO.width > collideable[i].gO.posX && ent.gO.posX < collideable[i].gO.posX + collideable[i].gO.width) {
                                                ent.gO.posY = collideable[i].gO.posY - ent.gO.height;
                                                if((collideable[i] == player && (ent.harmful)) || ((collideable[i].harmful) && ent == player)) {
                                                        player.yVel = -(10 + Math.random() * 5);
                                                        player.xVel = (Math.round(Math.random() * 1) == 1 ? -(10 + Math.random() * 5) : 10 + Math.random() * 5);
                                                        player.takeHealth(10);
                                                } else
                                                        ent.yVel = 0;
                                                rtn = collideable[i];
                                                ent.onGround = true;
                                        }
                                }
                        }
                } else {
                        ent.onGround = false;
                }
        }

        ent.gO.posX += ent.xVel;
        ent.gO.posY += ent.yVel;
        delete collideable;
        return rtn;
}
