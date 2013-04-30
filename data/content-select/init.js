
if(!("surface" in this))
    engine.abort("This program requires a graphical surface.");
surface.enableRepaintDebug();

var blackBlock = new GraphicsObject();

var boardEntryMap = {};
var boardEntries = [];
function boardEntry(code, title, pages) {
    this.element = new GraphicsContainer();
    this.element.setContainer(surface);
    this.element.setBackground("white");
    this.element.setBorder("black");

    var titleElement = new GraphicsText(title, Font("Arial", 11), this.element);
    var codeElement = new GraphicsText("/" + code + "/", {"family": "Arial", "size": 8}, this.element);

    engine.debug(titleElement.size());
    engine.debug(codeElement.size());
    this.element.resized.connect(function(size){
        titleElement.setPos(size.width / 2 - titleElement.width()/2, 2);
        codeElement.setPos(size.width / 2 - codeElement.width()/2, titleElement.height() -2);
    });

    this.element.setSize(150, 45);

    this.element.mousePress.connect(function(){
        openThreadList(code);
    });

    engine.debug("Added " + title);
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
        surface.setBackground("royal blue");
        fixBoardLayout();
    });

    var redBlock = new GraphicsObject(surface);
    redBlock.background = "red";
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

surface.resized.connect(fixBoardLayout);

updateBoardList();
