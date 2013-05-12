function setTimeout(callback, delay) {return engine.setTimeout(callback, delay);}
function clearTimeout(handle) {engine.clearTimeout(handle);}

function Noop(){}

Math.clamp = function(min, val, max){
    return Math.max(min, Math.min(max, val))
}

function Font(family, size) {
    return {"family": family, "size": size};
}

function Point(x, y){
    return {"x": x, "y": y};
}

function Size(width, height){
    return {"width": width, "height": height};
}

function Rect(x, y, width, height){
    return {"x": x, "y": y, "width": width, "height": height};
}

function Rgb(r, g, b) {
    return {"red": r, "green": g, "blue": b};
}

function Rgba(r, g, b, a) {
    return {"red": r, "green": g, "blue": b, "alpha": a};
}

function GradientStop(stop, color) {
    return [stop, color];
}

function Gradient(type, stops) {
    return {"type": type, "stops": stops};
}

function RadialGradient() {
    return Gradient("radial", $A(arguments));
}

function LinearGradient(stops, stopPos, startPos) {
    if(startPos == undefined)
        startPos = Point(0, 0);
    if(stopPos == undefined)
        stopPos = Point(0, 1);
    var gradient = Gradient("linear", stops);
    gradient.start = startPos;
    gradient.stop = stopPos;
    return gradient;
}

function VLinearGradient() {
    return LinearGradient($A(arguments));
}

function HLinearGradient() {
    return LinearGradient($A(arguments), Point(1, 0));
}

function Signal() {
    this.connections = [];
    this.emit = function() {
        var args = arguments;
        this.connections.forEach(function(handler){
            handler.apply(this, args);
        });
    }
    this.connect = function(handler) {
        if(this.connections.indexOf(handler) === -1)
            this.connections.push(handler);
    }
}

function requestJSON(url, callback) {
    var download = new ResourceRequest(url);
    download.receivedJSON.connect(callback);
}

function GroupResourceRequest(resources) {
    var thisObject = this;
    this.resourceList = resources;
    this.resourceData = {};
    var totalDownloads = 0;

    this.resourceList.forEach(function(res){
        thisObject.resourceData[res] = "";
        var download = new ResourceRequest(res);
        download.completed = function(data) {
            thisObject.oneCompleted.emit();
            thisObject.resourceData[res] = data;
            totalDownloads--;
            if(totalDownloads <= 0)
                thisObject.completed.emit(thisObject.resourceData);
        }
        totalDownloads++;
    });

    this.complete = new Signal();
    this.progress = new Signal();
    this.oneCompleted = new Signal();
}

//Dummy Console
var Console = {
    supportsColor: false,
    supportsOutput: false,
    supportsMovingCursor: false,

    log: Noop,
    warn: Noop,
    error: Noop
}
