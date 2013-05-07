var surface = new GraphicsSurface("Image Viewer Example (MOE Game Engine v" + engine.version + ")", Size(800, 600));
var boardPage = new GraphicsContainer(surface);
var boardEntryMap = {};
var boardEntries = [];

function boardEntry(code, title, pages, sleepFade) {
    this.element = new GraphicsContainer(boardPage);
    this.element.objectName = "Board /" + code + "/";
    this.element.background = VLinearGradient(GradientStop(0, "gray88"), GradientStop(1, "white"));
    this.element.cursor = "pointer";
    this.element.border = "gray95";
    this.element.opacity = 0;
    var boardElement = this.element;
    setTimeout(function() {
        boardElement.animate("opacity", 1);
    }, sleepFade);

    var titleElement = new GraphicsText(title, Font("Arial", 11), this.element);
    title.objectName = "Board /" + code + "/ Title";
    var codeElement = new GraphicsText("/" + code + "/", {"family": "Arial", "size": 8}, this.element);
    codeElement.objectName = "Board /" + code + "/ Code";

    this.element.resized.connect(function(size){
        titleElement.setPos(size.width / 2 - titleElement.width/2, 2);
        codeElement.setPos(size.width / 2 - codeElement.width/2, titleElement.height + 2);
    });

    this.element.setSize(150, 45);
    this.element.mousePressed.connect(function(){
        engine.debug("Entering /" + code + "/");
    });
    this.element.mouseEntered.connect(function(){
        boardElement.animate("background", VLinearGradient(GradientStop(0, "gray88"), GradientStop(1, "white")), 2);
    });
    this.element.mouseLeft.connect(function(){
        boardElement.animate("background", VLinearGradient(GradientStop(0, "gray95"), GradientStop(1, "white")), 2);
    });
}

function fixBoardLayout(size) {
    var cols = Math.floor(size.width / 155);
    var xStart = (size.width - (cols * 155))/2;
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
        engine.debug("Got Board List");
        engine.debug(boardData);
        var sleepFadeIn = 0;
        boardData.boards.forEach(function(board){
            if(!boardEntryMap[board.board]) {
                var nBoardEntry = new boardEntry(board.board, board.title, board.pages, sleepFadeIn);
                boardEntryMap[board.board] = nBoardEntry;
                boardEntries.push(nBoardEntry);
                sleepFadeIn += 50;
            }
        });
        fixBoardLayout(boardPage.size());
        surface.animate("background", "royal blue", 30);
        engine.debug(surface.background);
    });
}

var control = new GraphicsContainer(surface);
control.objectName = "ControlPanel";
control.setGeometry(Rect(0,-194,surface.width,200));
control.background = VLinearGradient(GradientStop(0, Rgba(0, 0, 0, 160)), GradientStop(1, Rgba(0, 0, 0, 120)));
control.opacity = 0.6;

control.mouseEntered.connect(function(){
    control.animate("opacity", 1);
    control.animate("y", 0);
});
control.mouseLeft.connect(function(){
    control.animate("opacity", 0.6);
    control.animate("y", -194);
});

var activePage = boardPage;
surface.resized.connect(function(size) {
    control.width = size.width;
    activePage.setSize(size);
});
boardPage.resized.connect(function(size){
    engine.debug(size);
    fixBoardLayout(size);
});
boardPage.setSize(surface.size());
surface.connected.connect(updateBoardList);

