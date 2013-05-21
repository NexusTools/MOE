var surface = new GraphicsSurface("Select Content (MOE Game Engine v" + engine.version + ")", Size(800, 600));

var moeLogo = new GraphicsImage(surface);
moeLogo.opacity = 0;

function Button(text) {
    this.button = new GraphicsText(text, Font("Arial", 10), surface);
    this.button.background = Rgba(54, 72, 77, 220);
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
        this.button.animate("background", Rgba(97, 121, 129, 220), 2);
        this.button.animate("foreground", Rgb(203, 236, 255), 2);
    }.bind(this));
    this.button.mouseLeft.connect(function(){
        if(this.selected)
            return;
        this.button.animate("background", Rgba(54, 72, 77, 220), 2);
        this.button.animate("foreground", Rgb(173, 216, 230), 2);
    }.bind(this));
    this.selectFade = (function(){
        if(!this.selected || this.button.isAnimating("background"))
            return;

        if(this.button.background.alpha < 150)
            this.button.animate("background", Rgba(173, 216, 230, 160), this.selectFade, 8);
        else
            this.button.animate("background", Rgba(173, 216, 230, 140), this.selectFade, 8);
    }).bind(this);
    this.setSelected = function(sel) {
        if(sel == this.selected)
            return;
        if(sel) {
            this.button.animate("background", Rgba(173, 216, 230, 160), this.selectFade);
            this.button.animate("foreground", Rgb(203, 236, 255), 2);
        } else {
            this.button.animate("background", Rgba(54, 72, 77, 220), 2);
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
        try {
            this.buttons.forEach(function(btn){
                btn.button.deleteLater();
            });
        }catch(e){}
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
            this.buttonChanged.emit(this.selectedButton.button.text,
                          this.buttons.indexOf(this.selectedButton));
        } else
            this.buttonChanged.emit(false, -1);
    }
    this.push = function(btn) {
        if(btn.button.width > this.width) {
            this.width = btn.button.width;
            this.buttons.forEach(function(bnt){
                bnt.button.width = this.width;
            }.bind(this));
        } else
            btn.button.width = this.width;
        btn.button.resized.connect(function(size){
            this.width = 0;
            this.buttons.forEach(function(btn){
                if(btn.button.width > this.width)
                    this.width = btn.button.width;
            }.bind(this));
            this.buttons.forEach(function(btn){
                btn.button.width = this.width;
            }.bind(this));
        }.bind(this));
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
    if(subMenuButtons) {
        subMenuButtons.animate("posX", 0);
        subMenuButtons.animate("opacity", 0);
    }

    moeLogo.animate("opacity", 0);
    leftButtons.animate("opacity", 0);
    leftButtons.animate("posX", -leftButtons.width);
    rightButtons.animate("opacity", 0);
    rightButtons.animate("posX", surface.width);
    surface.setTitle("Changing content...");
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
    loading.setPos(0, selPos.y + 4);
    loading.animate("posX", 10, 7);
    var exampleEntries = ResourceRequest(path);
    function createSubmenu(generator, callback) {
        subMenuButtons = new ButtonGroup({"opacity": 0});
        generator(subMenuButtons);
        var y = selPos.y + 4;
        if(loading)
            y += loading.height + 5;
        subMenuButtons.setPos(0, y);
        subMenuButtons.animate("posX", 14, 7, 120);
        subMenuButtons.animate("opacity", 1, 7, 120);
        if(loading)
            subMenuButtons.buttonChanged.connect(function(text, index){
                callback(text, index);
                if(!subMenuButtons)
                    loading.animate("opacity", 0);
            });
        else
            subMenuButtons.buttonChanged.connect(callback);
    }

    exampleEntries.receivedChildList.connect(function(children){
        if(!children.length) {
            loading.foreground = "red";
            loading.text = "No entries";

            createSubmenu(function(subMenuButtons){
                subMenuButtons.push(new Button("Retry"));
                subMenuButtons.push(new Button("<<"));
            }, function(btn, index) {
                if(index == 0) {
                    fadeDestroySubmenu();
                    loadChildSelectionSubmenu(path);
                } else
                    destroySubmenu();
            });
        } else {
            loading.animate("opacity", 0);
            loading = false;

            createSubmenu(function(subMenuButtons){
                children.each(function(child){
                    var btn = new Button(child);
                    var xmlPath = Url("info.xml", Url(child, path));
                    engine.debug(xmlPath);
                    var nameRequest = ResourceRequest(xmlPath);
                    nameRequest.receivedXML.connect(function(xmlData){
                        try {
                            btn.button.text = xmlData.MoeContent.ContentInfo.Name['#text'];
                        } catch(e) {}
                    });
                    subMenuButtons.push(btn);
                });
                subMenuButtons.push(new Button("<<"));
            }, function(btn, index){
                if(index >= children.length)
                    destroySubmenu();
                else
                    changeContent(Url(children[index], path));
            });
        }
    });
    exampleEntries.error.connect(function(error){
        loading.foreground = "red";
        loading.text = error;

        createSubmenu(function(subMenuButtons){
            subMenuButtons.push(new Button("Retry"));
            subMenuButtons.push(new Button("<<"));
        }, function(btn, index) {
            if(index == 0) {
                fadeDestroySubmenu();
                loadChildSelectionSubmenu(path);
            } else
                destroySubmenu();
        });
    });
}

var subMenuButtons = false;
function fadeDestroySubmenu() {
    subMenuButtons.setDisabled(true);
    subMenuButtons.animate("posX", 0, 4, 80, subMenuButtons.deleteLater);
    subMenuButtons.animate("opacity", 0, 4, 80, subMenuButtons.deleteLater);
    subMenuButtons = false;
}

function destroySubmenu() {
    fadeDestroySubmenu();

    leftButtons.setDisabled(false);
    leftButtons.animate("posX", 5, 4, 50);
    leftButtons.animate("opacity", 1, 4, 50);
}

leftButtons.buttonChanged.connect(function(btn){
    if(subMenuButtons)
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
    leftButtons.animate("posX", -8, 4, 50);
    leftButtons.animate("opacity", 0.4, 4, 50);
});

var rightButtons = new ButtonGroup({"opacity": 0});
rightButtons.push(new Button("Login"));
rightButtons.push(new Button("Register"));
rightButtons.push(new Button("NexusTools.net"));
rightButtons.setPos(0, 5);

rightButtons.buttonChanged.connect(function(btn, index){
    switch(index) {
    case 2:
        surface.openUrl("http://nexustools.net/");
        break;
    }
});

function start(size) {
    if(size.width < 1 || size.height < 1)
        return;
    var moeLogoDownload = ResourceRequest(Url("../resources/logo.png"));
    moeLogoDownload.receivedData.connect(function(data){
        moeLogo.load(data);
        moeLogo.animate("opacity", 1, 30);
    });
    moeLogoDownload.completed.connect(moeLogoDownload.deleteLater);

    surface.resized.disconnect(start);
    leftButtons.animate("posX", 5, 5, 80);
    leftButtons.animate("opacity", 1, 8, 80);
    defaultBackgroundColor = surface.background;
    surface.animate("background", "black", 30);
    rightButtons.setPos(size.width - rightButtons.width + 95, 5);
    rightButtons.animate("posX", size.width - rightButtons.width - 5, 5, 80);
    rightButtons.animate("opacity", 1, 8, 80);
    moeLogo.setSize(size);

    surface.resized.connect(function(size){
        moeLogo.setSize(size);
        rightButtons.setPos(size.width - rightButtons.width - 5, 5);
    });
}

surface.resized.connect(start);

engine.debug(surface.background);
