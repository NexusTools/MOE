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
    this.setSelected = function(sel) {
        if(sel == this.selected)
            return;
        if(sel) {
            this.button.animate("background", Rgba(173, 216, 230, 160), 2);
            this.button.animate("foreground", Rgb(203, 236, 255), 2);
        } else {
            this.button.animate("background", Rgba(173, 216, 230, 60), 2);
            this.button.animate("foreground", Rgb(173, 216, 230), 2);
        }

        this.selected = sel;
    }

    return this;
}

function ButtonGroup(expand) {
    this.buttons = $A();
    this.groupWidth = 0;
    this.selectedButton = false;
    this.selectButton = function(btn) {
        if(this.selectedButton)
            this.selectedButton.setSelected(false);
        this.selectedButton = btn;
        this.selectedButton.setSelected(true);
        this.buttonChanged.emit(this.selectedButton.button.text);
    }
    this.push = function(btn) {
        if(btn.button.width > this.groupWidth) {
            this.groupWidth = btn.button.width;
            this.buttons.forEach(function(bnt){
                bnt.button.width = this.groupWidth;
            }.bind(this));
        } else
            btn.button.width = this.groupWidth;

        btn.button.mousePressed.connect(function(){
            this.selectButton(btn);
        }.bind(this));
        this.buttons.push(btn);
        if(expand)
            btn.button.opacity = 0;
    }

    this.buttonChanged = new Signal();
}

var rightButtons = new ButtonGroup();
rightButtons.push(new Button("Gallery"));
rightButtons.push(new Button("Examples"));
rightButtons.push(new Button("Downloaded"));
rightButtons.push(new Button("Content Editor"));
rightButtons.push(new Button("Settings"));
rightButtons.push(new Button("Quit"));
var sumenuButtons = false;
rightButtons.buttonChanged.connect(function(btn){
    if(sumenuButtons) {
        sumenuButtons.destroy();
        sumenuButtons = false;
    }
    switch(btn) {
    case "Examples":
        var exampleEntries = ResourceRequest(":/examples/");
        exampleEntries.receivedChildList.connect(function(children){
            if(!children.length)
                throw "Examples missing...";
            engine.debug(children);
        });
        exampleEntries.error.connect(function(error){
            throw error;
        });
        break;

    case "Quit":
        engine.quit();

    default:
        throw "`" + btn + "` isn't implemented yet.";
    }
});

surface.resized.connect(function(size){
    var x = 5;
    var y = 5;
    rightButtons.buttons.forEach(function(btn){
        btn.button.setPos(x, y);
        y += btn.button.height + 5;
    });
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
