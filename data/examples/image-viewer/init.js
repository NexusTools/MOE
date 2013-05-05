
var surface = new GraphicsSurface("Image Viewer Example (MOE Game Engine v" + engine.version + ")", Size(800, 600));

//surface.enableRepaintDebug(false);
//engine.setTicksPerSecond(60);

var boardPage = new GraphicsContainer(surface);
var boardEntryMap = {};
var boardEntries = [];

function boardEntry(code, title, pages) {
    this.element = new GraphicsContainer(boardPage);
    this.element.objectName = "Board /" + code + "/";
    this.element.background = "white";
    this.element.border = "gray70";
    this.element.cursor = "pointer";

    var boardElement = this.element;
    var titleElement = new GraphicsText(title, Font("Arial", 11), this.element);
    title.objectName = "Board /" + code + "/ Title";
    var codeElement = new GraphicsText("/" + code + "/", {"family": "Arial", "size": 8}, this.element);
    codeElement.objectName = "Board /" + code + "/ Code";

    engine.debug(titleElement.size());
    engine.debug(codeElement.size());
    this.element.resized.connect(function(size){
        titleElement.setPos(size.width / 2 - titleElement.width/2, 2);
        codeElement.setPos(size.width / 2 - codeElement.width/2, titleElement.height + 2);
    });

    this.element.setSize(150, 45);
    this.element.mousePressed.connect(function(){
        engine.debug("Entering /" + code + "/");
    });
    this.element.mouseEntered.connect(function(){
        boardElement.animate("background", "gray90", 2);
    });
    this.element.mouseLeft.connect(function(){
        boardElement.animate("background", "white", 2);
    });
}

function fixBoardLayout() {
    var cols = Math.floor(surface.width / 155);
    var xStart = (surface.width - (cols * 155))/2;
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
    download.receivedJSON.connect(function(boardData){
        surface.animate("background", "royal blue");
        engine.debug("Got Board List");
        engine.debug(boardData);
        boardData.boards.forEach(function(board){
            if(!boardEntryMap[board.board]) {
                var nBoardEntry = new boardEntry(board.board, board.title, board.pages);
                boardEntryMap[board.board] = nBoardEntry;
                boardEntries.push(nBoardEntry);
            }
        });
        fixBoardLayout();
    });
}

var control = new GraphicsContainer(surface);
control.objectName = "ControlPanel";
control.setGeometry(Rect(0,-190,surface.width,200));
control.background = Rgba(0,0,0,160);
control.opacity = 0.1;

control.mouseEntered.connect(function(){
    control.animate("opacity", 1);
    control.animate("y", 0);
});
control.mouseLeft.connect(function(){
    control.animate("opacity", 0.1);
    control.animate("y", -190);
});

surface.resized.connect(function(size) {
    control.width = size.width;
    boardPage.setSize(size);
    fixBoardLayout();
});
boardPage.setSize(surface.size());
updateBoardList();

