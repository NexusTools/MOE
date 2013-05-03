
if(!("surface" in this))
    engine.abort("This program requires a graphical surface.");
//surface.enableRepaintDebug(false);
//engine.setTicksPerSecond(60);
surface.setTitle("Chan Browser");
surface.background = Rgba(0,0,0,0);

var blackBlock = new GraphicsObject();
var boardEntryMap = {};
var boardEntries = [];
function boardEntry(code, title, pages) {
    this.element = new GraphicsContainer(surface);
    this.element.objectName = "Board /" + code + "/";
    this.element.background = "white";
    this.element.border = "black";

    var boardElement = this.element;
    var titleElement = new GraphicsText(title, Font("Arial", 11), this.element);
    title.objectName = "Board /" + code + "/ Title";
    var codeElement = new GraphicsText("/" + code + "/", {"family": "Arial", "size": 8}, this.element);
    codeElement.objectName = "Board /" + code + "/ Code";

    engine.debug(titleElement.size());
    engine.debug(codeElement.size());
    this.element.resized.connect(function(size){
        titleElement.setPos(size.width / 2 - titleElement.width()/2, 2);
        codeElement.setPos(size.width / 2 - codeElement.width()/2, titleElement.height() -2);
    });

    this.element.setSize(150, 45);
    this.element.mousePressed.connect(function(){
    });
    this.element.mouseEntered.connect(function(){
        boardElement.background = "gray10";
    });
    this.element.mouseLeft.connect(function(){
        boardElement.background = "white";
    });
}

function fixBoardLayout() {
    var cols = Math.floor(surface.width() / 155);
    var xStart = (surface.width() - (cols * 155))/2;
    var colX = 0;
    var y = 5;
    boardEntries.forEach(function(board) {
        board.element.setPos(Point(xStart + colX*155 + 5, y));
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
        surface.background = "royal blue";/*Rgba(8, 8, 247, 50);*/
        fixBoardLayout();
    });
}

surface.resized.connect(fixBoardLayout);
updateBoardList();
