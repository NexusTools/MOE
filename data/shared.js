function Noop(){}

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

//Dummy Console
var Console = {
    supportsColor: false,
    supportsOutput: false,
    supportsMovingCursor: false,



    log: Noop,
    warn: Noop,
    error: Noop

}
