var titleMenuButtonForgroundColor = Rgba(255, 255, 255, 200);
var titleMenuButtonForgroundColorHovered = Rgba(255, 255, 255, 255);
var titleMenuButtonForgroundColorActive = Rgba(80, 0, 0, 255);
var titleMenuFont = Font("monospace", 26);

engine.setTicksPerSecond(30);
var surface = new GraphicsSurface("BlockCity", Size(800, 600));
surface.background = Rgb(50, 0, 0);

var curScreen = null;
var curLevel = null;

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
            curLevel.enemy[curLevel.enemy.length] = new Entity(point.x, point.y, 50, 75, "white");
        }
    });

    this.handleResize = function(size) {
        this.gC.setSize(size);
    }
}

function Level() {
    this.enemy = [];
    this.block = [];
    this.block[0] = new Entity(0, 0, 25, 800, "grey");
}

function Entity(x, y, width, height, col) {
    engine.debug("x: " + x + ", y: " + y + ", w: " + width + ", h: " + height + ", col: " + col);
    this.gO = new GraphicsObject(curScreen.gC);
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

//engine.tick.connect(func);

function processPhysics(ent, doBlocks, doEn, doPly) {
        var rtn;
        var collideable2 = [];
        collideable2 = collideable2.concat((doBlocks ? curLevel.block : []), (doEn ? curLevel.enemy : []), (doPly ? player : []));
        var collideable = [];
        var i2 = 0;
        var nx = ent.rect.x + ent.xVel;
        var ny = ent.rect.y + ent.yVel;
        var nw = ent.rect.width * 2;
        var nh = ent.rect.height * 2;
        for(var i = 0; i < collideable2.length; i++) {
                if(collideable2[i] == ent)
                        continue;
                if(nx + nw >= collideable2[i].rect.x && nx - (nw / 2) <= collideable2[i].rect.x + collideable2[i].rect.width && ny + nh >= collideable2[i].rect.y && ny - (nh / 2) <= collideable2[i].rect.y + collideable2[i].rect.height)
                        collideable[i2++] = collideable2[i];
        }
        delete nx;
        delete ny;
        delete nw;
        delete nh;
        delete i2;
        delete collideable2;

        if(!ent.onGround)
                if(ent.yVel < ent.rect.height)
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

        if(ent.rect.x + ent.xVel < 0) {
                ent.rect.x = 0;
                ent.xVel = 0;
                rtn = 1;
        } else if(ent.rect.x + ent.rect.width + ent.xVel > mainScene.width) {
                ent.rect.x = mainScene.width - ent.rect.width;
                ent.xVel = 0;
        } else {
                if(ent.xVel != 0) {
                        for(var i = 0; i < collideable.length; i++) {
                                if(ent.xVel > 0) {
                                        if(ent.rect.x + ent.rect.width + ent.xVel > collideable[i].rect.x && ent.rect.x + ent.xVel < collideable[i].rect.x && ent.rect.y + ent.rect.height > collideable[i].rect.y && ent.rect.y < collideable[i].rect.y + collideable[i].rect.height) {
                                                ent.rect.x = collideable[i].rect.x - ent.rect.width;
                                                if((collideable[i] == player && (ent instanceof DumbEnemy || ent instanceof CrazyEnemy)) || ((collideable[i] instanceof DumbEnemy || collideable[i] instanceof CrazyEnemy) && ent == player)) {
                                                        player.xVel = 10 + Math.random() * 5;
                                                        player.yVel = -(10 + Math.random() * 5);
                                                        player.takeHealth(10);
                                                }
                                                ent.xVel = 0;
                                                rtn = (ent == player ? collideable[i] : 2);
                                        }
                                } else if(ent.xVel < 0) {
                                        if(ent.rect.x + ent.rect.width + ent.xVel > collideable[i].rect.x + collideable[i].rect.width && ent.rect.x + ent.xVel < collideable[i].rect.x + collideable[i].rect.width && ent.rect.y + ent.rect.height > collideable[i].rect.y && ent.rect.y < collideable[i].rect.y + collideable[i].rect.height) {
                                                ent.rect.x = collideable[i].rect.x + collideable[i].rect.width;
                                                if((collideable[i] == player && (ent instanceof DumbEnemy || ent instanceof CrazyEnemy)) || ((collideable[i] instanceof DumbEnemy || collideable[i] instanceof CrazyEnemy) && ent == player)) {
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

        if(ent.rect.y + ent.yVel < 0) {
                ent.rect.y = 0;
                ent.yVel = 0;
        } else if(ent.rect.y + ent.rect.height + ent.yVel > mainScene.height) {
                ent.rect.y = mainScene.height - ent.rect.height;
                ent.yVel = 0;
                ent.onGround = true;
        } else {
                if(ent.yVel != 0) {
                        for(i = 0; i < collideable.length; i++) {
                                if(ent.yVel < 0) {
                                        if(ent.rect.y + ent.yVel + ent.rect.height > collideable[i].rect.y + collideable[i].rect.height && ent.rect.y + ent.yVel < collideable[i].rect.y + collideable[i].rect.height && ent.rect.x + ent.rect.width > collideable[i].rect.x && ent.rect.x < collideable[i].rect.x + collideable[i].rect.width) {
                                                ent.rect.y = collideable[i].rect.y + collideable[i].rect.height;
                                                if((collideable[i] == player && (ent instanceof DumbEnemy || ent instanceof CrazyEnemy)) || ((collideable[i] instanceof DumbEnemy || collideable[i] instanceof CrazyEnemy) && ent == player)) {
                                                        player.xVel = (Math.round(Math.random() * 1) == 1 ? -(10 + Math.random() * 5) : 10 + Math.random() * 5);
                                                        player.takeHealth(10);
                                                } else
                                                        ent.yVel = 0;
                                                rtn = collideable[i];
                                        }
                                } else if(ent.yVel > 0) {
                                        if(ent.rect.y + ent.yVel + ent.rect.height > collideable[i].rect.y && ent.rect.y + ent.yVel < collideable[i].rect.y && ent.rect.x + ent.rect.width > collideable[i].rect.x && ent.rect.x < collideable[i].rect.x + collideable[i].rect.width) {
                                                ent.rect.y = collideable[i].rect.y - ent.rect.height;
                                                if((collideable[i] == player && (ent instanceof DumbEnemy || ent instanceof CrazyEnemy)) || ((collideable[i] instanceof DumbEnemy || collideable[i] instanceof CrazyEnemy) && ent == player)) {
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

        ent.rect.x += ent.xVel;
        ent.rect.y += ent.yVel;
        delete collideable;
        return rtn;
}
