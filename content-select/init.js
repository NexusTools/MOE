var surface = new GraphicsSurface("Select Content (MOE Game Engine v" + engine.version + ")", Size(800, 600));

var snakes = [];
var moeLogo = new GraphicsImage(Url("../resources/logo.png"));

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

function Button(text) {
    this.button = new GraphicsText(text, Font("Arial", 10), surface);
    this.button.background = Rgba(173, 216, 230, 60);
    this.button.foreground = Rgb(173, 216, 230);
    this.button.border = Rgb(173, 216, 230);
    this.button.cursor = "pointer";
    this.button.borderRadius = 3;
    this.button.margin = 5;
    this.selFade = false;
    this.selected = false;
    this.button.mouseEntered.connect(function(){
        if(this.selected)
            return;
        this.button.animate("background", Rgba(173, 216, 230, 110), 2);
        this.button.animate("foreground", Rgb(203, 236, 255), 2);
    }.bind(this));
    this.button.mouseLeft.connect(function(){
        if(this.selected)
            return;
        this.button.animate("background", Rgba(173, 216, 230, 60), 2);
        this.button.animate("foreground", Rgb(173, 216, 230), 2);
    }.bind(this));
    this.selectFade = (function(){
        if(!this.selected || this.button.isAnimating("background"))
            return;

        if(this.button.background.alpha < 150)
            this.button.animate("background", Rgba(173, 216, 230, 160), this.selectFade);
        else
            this.button.animate("background", Rgba(173, 216, 230, 140), this.selectFade);
    }).bind(this);
    this.setSelected = function(sel) {
        if(sel == this.selected)
            return;
        if(sel) {
            this.button.animate("background", Rgba(173, 216, 230, 160), this.selectFade);
            this.button.animate("foreground", Rgb(203, 236, 255), 2);
        } else {
            this.button.animate("background", Rgba(173, 216, 230, 60), 2);
            this.button.animate("foreground", Rgb(173, 216, 230), 2);
        }
        this.selected = sel;
    }

    return this;
}

function ButtonGroup(defaultProperties) {
    this.width = 0;
    this.buttons = $A();
    this.selectedButton = false;
    this.props = $H(defaultProperties);
    this.deleteLater = function() {
        this.buttons.forEach(function(btn){
            btn.button.deleteLater();
        });
    }

    this.getSelectedButtonPos = function() {
        return this.selectedButton ? this.selectedButton.button.pos : Point(0, 0);
    }
    this.selectButton = function(btn) {
        if(this.selectedButton)
            this.selectedButton.setSelected(false);
        this.selectedButton = btn;
        if(this.selectedButton) {
            this.selectedButton.setSelected(true);
            this.buttonChanged.emit(this.selectedButton.button.text);
        } else
            this.buttonChanged.emit(false);
    }
    this.push = function(btn) {
        if(btn.button.width > this.width) {
            this.width = btn.button.width;
            this.buttons.forEach(function(bnt){
                bnt.button.width = this.width;
            }.bind(this));
        } else
            btn.button.width = this.width;
        this.props.each(function(pair){
            engine.debug(pair);
            btn.button[pair.key] = pair.value;
        });

        btn.button.mousePressed.connect(function(){
            this.selectButton(btn);
        }.bind(this));
        this.buttons.push(btn);
    }

    this.animate = function(prop, to, mod, delay, callback) {
        var delayTime = 0;
        var lastButton = this.buttons[this.buttons.length-1];
        this.buttons.forEach(function(btn){
            var last = btn == lastButton;
            if(delay > 0) {
                setTimeout(function(){
                    if(last && typeof callback == "function")
                        btn.button.animate(prop, to, callback, mod)
                    else
                        btn.button.animate(prop, to, mod)
                }, delayTime);
                delayTime += delay;
            } else if(last && typeof callback == "function")
                btn.button.animate(prop, to, callback, mod)
            else
                btn.button.animate(prop, to, mod);
        });
    }

    this.setPos = function(x, y) {
        this.buttons.forEach(function(btn){
            btn.button.setPos(x, y);
            y += btn.button.height + 5;
        });
    }

    this.setDisabled = function(dis) {
        this.buttons.forEach(function(btn){
            btn.button.setDisabled(dis);
        });
    }

    this.buttonChanged = new Signal();
}

var leftButtons = new ButtonGroup({"opacity": 0});
leftButtons.push(new Button("Gallery"));
leftButtons.push(new Button("Examples"));
leftButtons.push(new Button("Downloaded"));
leftButtons.push(new Button("Content Editor"));
leftButtons.push(new Button("Settings"));
leftButtons.push(new Button("Quit"));
leftButtons.setPos(-leftButtons.width, 5);

function changeContent(url) {
    if(sumenuButtons)
        destroySubmenu();
    leftButtons.animate("opacity", 0);
    rightButtons.animate("opacity", 0);
    engine.tick.connect(function(){
        if(snakePenColor.alpha > 0) {
            snakePenColor.alpha /= 4;
            surface.foreground = snakePenColor;
        }
    });
    surface.animate("background", defaultBackgroundColor, function() {
        engine.startContent(url);
    });
}

function loadChildSelectionSubmenu(path) {
    path = Url(path);
    var loading = new GraphicsText("Loading...", Font("Arial", 10), surface);
    loading.margin = 4;
    loading.foreground = "white";
    loading.background = Rgba(0, 0, 0, 80);
    var selPos = leftButtons.getSelectedButtonPos();
    loading.setPos(5, selPos.y);
    var exampleEntries = ResourceRequest(path);
    exampleEntries.receivedChildList.connect(function(children){
        if(!children.length) {
            loading.foreground = "red";
            loading.text = "No entries";
        } else {
            loading.animate("opacity", 0, function(){
                loading.deleteLater();
            });
            sumenuButtons = new ButtonGroup({"opacity": 0});
            children.each(function(path){
                sumenuButtons.push(new Button(path));
            });
            sumenuButtons.push(new Button("<<"));
            sumenuButtons.setPos(5, selPos.y);
            sumenuButtons.animate("posX", 10, 3, 80);
            sumenuButtons.animate("opacity", 1, 3, 80);
            sumenuButtons.buttonChanged.connect(function(btn){
                switch(btn) {
                case "<<":
                    destroySubmenu();
                    break;

                default:
                    changeContent(Url(btn, path));
                }
            });
        }
    });
    exampleEntries.error.connect(function(error){
        loading.foreground = "red";
        loading.text = error;
    });
}

var sumenuButtons = false;
function fadeDestroySubmenu() {
    sumenuButtons.setDisabled(true);
    sumenuButtons.animate("opacity", 0, 4, 50, sumenuButtons.deleteLater);
    sumenuButtons = false;
}

function destroySubmenu() {
    fadeDestroySubmenu();

    leftButtons.setDisabled(false);
    leftButtons.animate("posX", 5);
    leftButtons.animate("opacity", 1);
}

leftButtons.buttonChanged.connect(function(btn){
    if(sumenuButtons)
        fadeDestroySubmenu();

    switch(btn) {
    case "Gallery":
        loadChildSelectionSubmenu("http://moe.nexustools.net/gallery/");
        break;

    case "Examples":
        loadChildSelectionSubmenu("../examples/");
        break;

    case "Downloaded":
        loadChildSelectionSubmenu("storage:/downloaded-content/by-category/");
        break;

    case "Content Editor":
        changeContent("http://moe.nexustools.net/packages/nexustools/tools/content-editor/");
        return;

    case "Settings":
        changeContent("settings:/");
        return;

    case "Quit":
        engine.quit();

    default:
        throw "`" + btn + "` isn't implemented yet.";
    }

    leftButtons.setDisabled(true);
    leftButtons.animate("posX", -5);
    leftButtons.animate("opacity", 0.4);
});

var rightButtons = new ButtonGroup({"opacity": 0});
rightButtons.push(new Button("Login"));
rightButtons.push(new Button("Register"));
rightButtons.push(new Button("NexusTools.net"));
rightButtons.setPos(0, 5);

function start(size) {
    if(size.width < 1 || size.height < 1)
        return;
    surface.resized.disconnect(start);
    leftButtons.animate("posX", 5, 5, 80);
    leftButtons.animate("opacity", 1, 8, 80);
    defaultBackgroundColor = surface.background;
    surface.animate("background", "black", 30);
    rightButtons.setPos(size.width - rightButtons.width + 95, 5);
    rightButtons.animate("posX", size.width - rightButtons.width - 5, 5, 80);
    rightButtons.animate("opacity", 1, 8, 80);

    surface.resized.connect(function(size){
        rightButtons.setPos(size.width - rightButtons.width - 5, 5);
    });

    spawnSnakes();
}

surface.resized.connect(start);

engine.debug(surface.background);
