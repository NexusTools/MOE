
if(!("surface" in this))
    engine.abort("This program requires a graphical surface.");
//surface.enableRepaintDebug(false);
surface.setTitle("Chan Browser");

var blackBlock = new GraphicsObject();

var boardEntryMap = {};
var boardEntries = [];
function boardEntry(code, title, pages) {
    this.element = new GraphicsContainer();
    this.element.setContainer(surface);
    this.element.background = "white";
    this.element.border = "black";

    var boardElement = this.element;
    var titleElement = new GraphicsText(title, Font("Arial", 11), this.element);
    var codeElement = new GraphicsText("/" + code + "/", {"family": "Arial", "size": 8}, this.element);

    engine.debug(titleElement.size());
    engine.debug(codeElement.size());
    this.element.resized.connect(function(size){
        titleElement.setPos(size.width / 2 - titleElement.width()/2, 2);
        codeElement.setPos(size.width / 2 - codeElement.width()/2, titleElement.height() -2);
    });

    this.element.setSize(150, 45);
    this.element.mousePressed.connect(function(){
        //openThreadList(code);
    });
    this.element.mouseEntered.connect(function(){
        this.element.background = "royal blue";
    });
    this.element.mouseLeft.connect(function(){
        this.element.background = "white";
    });
}

function fixBoardLayout() {
    var cols = Math.floor(surface.width() / 160);
    var xStart = (surface.width() - (cols * 160))/2;
    var colX = 0;
    var y = 5;
    boardEntries.forEach(function(board) {
        board.element.setPos(Point(xStart + colX*160 + 5, y));
        colX++;
        if(colX >= cols) {
            y += 50;
            colX = 0;
        }
    });
}

function updateBoardList() {
    function block() {
        var redBlock = new GraphicsObject(surface);
        redBlock.background = "red";
        redBlock.border = "dark red";
        redBlock.setSize(50, 50);
        var x = 0;
        var y = 0;
        var xSpeed = 0;
        var ySpeed = 0;
        var xDirection = 0.1;
        var yDirection = 0.1;
        engine.tick.connect(function(){
            xSpeed += xDirection;
            ySpeed += yDirection;
            x += xSpeed;
            y += ySpeed;
            if(x < 0) {
                x = 0;
                xDirection = 0.1;
                xSpeed = -xSpeed/2;
            }
            if(y < 0) {
                y = 0;
                yDirection = 0.1;
                ySpeed = -ySpeed/2;
            }
            if(x + 50 > surface.width()) {
                x = surface.width() - 50;
                xDirection = -0.1;
                xSpeed = -xSpeed/2;
            }
            if(y + 50 > surface.height()) {
                y = surface.height() - 50;
                yDirection = -0.1;
                ySpeed = -ySpeed/2;
            }
            redBlock.setPos(x, y);
        });
    }

    var download = new ResourceRequest("https://api.4chan.org/boards.json");
    download.complete.connect(function(data){
        engine.debug("Got Board List");
        var boardData = engine.eval("(" + data + ")");
        boardData.boards.forEach(function(board){
            if(!boardEntryMap[board.board]) {
                var nBoardEntry = new boardEntry(board.board, board.title, board.pages);
                boardEntryMap[board.board] = nBoardEntry;
                boardEntries.push(nBoardEntry);
            }
        });
        var from = surface.background;
        surface.background = "royal blue";
        var to = surface.background;
        to = Rgb(to.red - from.red, to.green - from.green, to.blue - from.blue);
        surface.background = from;

        var fade = 0;
        function fadeCallback(){
            fade+=0.05;
            surface.background = Rgb(from.red + to.red*fade, from.green + to.green*fade, from.blue + to.blue*fade);
            if(fade >= 1) {
                engine.tick.disconnect(fadeCallback);

                block();
            }
        }
        engine.tick.connect(fadeCallback);

        fixBoardLayout();
    });



}

surface.resized.connect(fixBoardLayout);
surface.mouseMoved.connect(function(pos){engine.debug(pos);});
updateBoardList();
