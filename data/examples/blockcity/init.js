var wantedWidth = 800, wantedHeight = 600;

var collisions = 0;
var collisionsPerSecond = 0;
var collisionC = 0;
var collisionCPerSecond = 0;
var lastCollision = 0;

var paused = true;
var debug = false;
var pie = true;

var mousePos;

engine.setTicksPerSecond(30);
surface.background = Rgb(50, 0, 0);
var surface = new GraphicsSurface("BlockCity", Size(800, 600));
var mainScene = new GraphicsContainer(surface);

surface.resized.connect(function(size){
	console.debug(size);
	mainScene.setSize(surface.size());
});
mainScene.setSize(surface.size());

/*mainScene.restrictCamera = false;
mainScene.cameraSmooth = 5;*/
var curLevel = 0;
var level = [];
level[0] = new Level(500, 500, -16, -28);
level[1] = new Level(1000, 300, 40, 250 - 28);
level[2] = new Level(700, 300, 40, 125 - 28);

var player = new Player(new Color(200, 150, 0, 255));
var spectatingEnt = player;

level[0].block[0] = new Block(0, 0, 500, 25, new Color(0, 80, 140, 255));
level[0].block[1] = new Block(0, 500 - 25, 500, 25, new Color(0, 80, 140, 255));
level[0].block[2] = new Block(0, 25, 25, 450, new Color(0, 80, 140, 255));
level[0].block[3] = new Block(500 - 25, 25, 25, 950, new Color(0, 80, 140, 255));

level[1].block[0] = new Block(0, 25, 25, 225, new Color(80, 80, 80, 255));
level[1].block[1] = new Block(0, 0, 1000, 25, new Color(80, 80, 80, 255));
level[1].block[2] = new Block(0, 250, 100, 25, new Color(80, 80, 80, 255));
level[1].block[3] = new Block(100 - 25, 275, 500, 25, new Color(80, 80, 80, 255));
level[1].block[4] = new Block(550, 250, 50, 25, new Color(80, 80, 80, 255));
level[1].block[5] = new Block(575, 225, 50, 25, new Color(80, 80, 80, 255));
level[1].block[6] = new Block(600, 200, 50, 25, new Color(80, 80, 80, 255));
level[1].block[7] = new Block(625, 175, 375, 25, new Color(80, 80, 80, 255));
level[1].block[8] = new Block(1000 - 25, 25, 25, 150, new Color(80, 80, 80, 255));

level[2].block[0] = new Block(0, 0, 700, 25, new Color(0, 80, 0, 255));
level[2].block[1] = new Block(0, 25, 25, 100, new Color(0, 80, 0, 255));
level[2].block[2] = new Block(0, 125, 150, 25, new Color(0, 80, 0, 255));
level[2].block[3] = new Block(125, 150, 25, 150, new Color(0, 80, 0, 255));
level[2].block[4] = new Block(150, 275, 550, 25, new Color(0, 80, 0, 255));
level[2].block[5] = new Block(700 - 25, 25, 25, 250, new Color(0, 80, 0, 255));

level[0].enemy[0] = new DumbEnemy(25, 250, 16, 20, true);
level[0].enemy[1] = new DumbEnemy(50, 250, 16, 20, true);
level[0].enemy[2] = new DumbEnemy(75, 250, 16, 20, true);
level[0].enemy[3] = new DumbEnemy(100, 250, 16, 20, true);
level[0].enemy[4] = new CrazyEnemy(125, 250, 16, 20, true);
level[0].enemy[5] = new CrazyEnemy(150, 250, 16, 20, true);
level[0].enemy[6] = new CrazyEnemy(175, 250, 16, 20, true);
level[0].enemy[7] = new CrazyEnemy(200, 250, 16, 20, true);
level[0].enemy[8] = new CrazyEnemy(225, 250, 16, 20, true);
level[0].enemy[9] = new CrazyEnemy(250, 250, 16, 20, true);

level[1].enemy[0] = new DumbEnemy(300, 300 - 25 - 20, 16, 20, false);

level[2].enemy[0] = new CrazyEnemy(300, 300 - 25 - 20, 16, 20, false);

level[1].exitPortal[0] = new ExitPortal(875, 125, 2);

level[2].exitPortal[0] = new ExitPortal(580, 200, 1);

level[1].label[0] = new Label(50, 150, "Welcome!", new Color(0, 255, 0, 200));
level[1].label[1] = new Label(200, 175, "Watch out!\nThis monster is not nice and will try to kill you.", new Color(255, 255, 255, 200));
level[1].label[2] = new Label(700, 50, "This is a portal.\nI recommend you use it.", new Color(255, 255, 255, 200));

level[2].label[0] = new Label(100, 30, "Watch out...\n\nThis monster seems absolutely crazy.\nTry to dodge it.", new Color(255, 255, 255, 200));

var information = new GraphicsText();
information.font = "12px";
information.color = new Color(255, 255, 255, 200);
information.margin = 5;
add(information);

var healthbg = new GraphicsRect(0, 25, new Color(100, 0, 0, 200));
healthbg.y = 5;
var health = new GraphicsRect(0, 25, new Color(200, 0, 0, 200));
health.y = 5;
var healthText = new GraphicsText("Health", 0, 0, new Color(255, 255, 255, 200));
healthText.y = 5;
healthText.font = "20px";
healthText.color = new Color(255, 255, 255, 255);
add(healthbg);
add(health);
add(healthText);

var menuOverlay = new GraphicsRect(width, height, new Color(0, 0, 0, 100));

var defaultMenuColor = new Color(0, 0, 0, 200);

var startGame = new GraphicsText("Start Game", 0, 0);
startGame.color = new Color(255, 255, 255, 200);
startGame.margin = 10;
startGame.radius = 10;
startGame.brush = defaultMenuColor;

var levelEditor = new GraphicsText("Level Editor", 0, 0);
levelEditor.color = new Color(255, 255, 255, 200);
levelEditor.margin = 10;
levelEditor.radius = 10;
levelEditor.brush = defaultMenuColor;

var exitGame = new GraphicsText("Exit", 0, 0);
exitGame.color = new Color(255, 255, 255, 200);
exitGame.margin = 10;
exitGame.radius = 10;
exitGame.brush = defaultMenuColor;

var oAlpha = 0;
var dAlpha = false;
var old = toGraphic();
resize();
loadLevel(0, false);

installTickRoutine(function() {
	if(paused) {
		repaint();
		return;
	}
	
	player.process();
	updateCamera();
	
	for(var i = 0; i < level[curLevel].enemy.length; i++) {
		level[curLevel].enemy[i].process();
	}
	for(var i = 0; i < level[curLevel].enemyPortal.length; i++) {
		level[curLevel].enemyPortal[i].process();
	}
	for(var i = 0; i < level[curLevel].exitPortal.length; i++) {
		level[curLevel].exitPortal[i].process();
	}
	
	if((isKeyDown('`'.charCodeAt(0)) || isKeyDown('~'.charCodeAt(0)))) {
		if(!wasDownTilda)
			debug = !debug;
		wasDownTilda = true;
	} else
		wasDownTilda = false;
	
	if(isKeyDown(Event.Key_Escape) && curLevel != 0) {
		pie = true;
		loadLevel(0, true);
	}
	
	if(debug) {
		var T = new Date().getTime();
		if(T - lastCollision >= 1000) {
			lastCollision = T;
			collisionsPerSecond = collisions;
			collisionCPerSecond = collisionC;
			collisions = 0;
			collisionC = 0;
		}
		delete T;
	}
	if(player.rect.opacity < 1 && !player.dead) {
		player.rect.opacity+=0.1;
		if(player.rect.opacity >= 1) {
			player.rect.opacity = 1;
		}
	}
	
	if(curLevel == 0) {
		if(Math.round(Math.random() * 100) == 1 || pie) {
			spectatingEnt = level[curLevel].enemy[Math.round(Math.random() * (level[curLevel].enemy.length - 1))];
			pie = false;
		}
		if(mousePos) {
			if(doesIntersect(mousePos.x, mousePos.y, startGame.x, startGame.y, 1, 1, startGame.width, startGame.height)) {
				var lastColor = new Color(startGame.brush);
				var green = lastColor.green + 10;
				if(green >= 100)
					green = 100;
				startGame.brush = new Color(lastColor.red, green, lastColor.blue, lastColor.alpha);
			} else {
				var lastColor = new Color(startGame.brush);
				var green = lastColor.green - 10;
				if(green <= 0)
					green = 0;
				startGame.brush = new Color(lastColor.red, green, lastColor.blue, lastColor.alpha);
			}
			if(doesIntersect(mousePos.x, mousePos.y, levelEditor.x, levelEditor.y, 1, 1, levelEditor.width, levelEditor.height)) {
				var lastColor = new Color(levelEditor.brush);
				var green = lastColor.green + 10;
				if(green >= 100)
					green = 100;
				levelEditor.brush = new Color(lastColor.red, green, lastColor.blue, lastColor.alpha);
			} else {
				var lastColor = new Color(levelEditor.brush);
				var green = lastColor.green - 10;
				if(green <= 0)
					green = 0;
				levelEditor.brush = new Color(lastColor.red, green, lastColor.blue, lastColor.alpha);
			}
			if(doesIntersect(mousePos.x, mousePos.y, exitGame.x, exitGame.y, 1, 1, exitGame.width, exitGame.height)) {
				var lastColor = new Color(exitGame.brush);
				var green = lastColor.green + 10;
				if(green >= 100)
					green = 100;
				exitGame.brush = new Color(lastColor.red, green, lastColor.blue, lastColor.alpha);
			} else {
				var lastColor = new Color(exitGame.brush);
				var green = lastColor.green - 10;
				if(green <= 0)
					green = 0;
				exitGame.brush = new Color(lastColor.red, green, lastColor.blue, lastColor.alpha);
			}
		}
	}
	
	information.text =	(debug ? "Mouse X: " + (mousePos ? Math.round(mousePos.x) : 0) + ", Y: " + (mousePos ? Math.round(mousePos.y) : 0) + ".\n" +
						"Player X: " + Math.round(player.rect.x) + ", Y: " + Math.round(player.rect.y) + ".\n" +
						"Enemies: " + level[curLevel].enemy.length + ", Blocks: " + level[curLevel].block.length + ".\n" +
						"Collisions per second: " + collisionsPerSecond + ".\n" +
						"Collision checks per second: " + collisionCPerSecond + ".\n" +
						"FPS: " + Math.round(fps()) + ".\n" +
						"Tick Load: " + Math.round(tickLoad() * 100) + "%, Paint Load: " + Math.round(paintLoad() * 100) + "%.\n\n" : "") +
						(curLevel != 0 ? "Press ESC to exit to menu." : "");
	information.update();
});

installResizeListener(resize);

function resize() {
	menuOverlay.width = width;
	menuOverlay.height = height;
	startGame.x = width / 2 - (startGame.width / 2);
	startGame.y = height / 2 - (startGame.height / 2) - 40;
	levelEditor.x = width / 2 - (levelEditor.width / 2);
	levelEditor.y = height / 2 - (levelEditor.height / 2);
	exitGame.x = width / 2 - (exitGame.width / 2);
	exitGame.y = height / 2 - (exitGame.height / 2) + 40;
	updateHealth();
	updateCamera();
	repaint();
}

function updateCamera() {
	if(spectatingEnt) {
		mainScene.cameraX = Math.round(spectatingEnt.rect.x - (width / 2) + (spectatingEnt.rect.width / 2));
		mainScene.cameraY = Math.round(spectatingEnt.rect.y - (height / 2) + (spectatingEnt.rect.height / 2));
	}
}

function updateHealth() {
	healthbg.width = (width / 1.5);
	healthbg.x = width / 2 - (healthbg.width / 2);
	health.width = (player.health / 100 * healthbg.width);
	health.x = width / 2 - (health.width / 2);
	healthText.x = width / 2 - healthText.width / 2
}

function loadLevel(l, transition) {
	if(l != 0 && curLevel == 0) {
		removeGraphic(menuOverlay);
		removeGraphic(startGame);
		removeGraphic(levelEditor);
		removeGraphic(exitGame);
	} else if(l == 0) {
		add(menuOverlay);
		add(startGame);
		add(levelEditor);
		add(exitGame);
	}
	print("Loading level " + l);
	player.inPortal = false;
	if(transition) {
		this.paused = true;
		this.oAlpha = 0;
		this.dAlpha = false;
		this.old = this.toGraphic();
		installPaintRoutine(doAnimation);
	}
	for(var i = 0; i < level[curLevel].label.length; i++) {
		mainScene.removeGraphic(level[curLevel].label[i].textObj);
	}
	for(var i = 0; i < level[curLevel].enemy.length; i++) {
		mainScene.removeGraphic(level[curLevel].enemy[i].rect);
	}
	for(var i = 0; i < level[curLevel].enemyPortal.length; i++) {
		mainScene.removeGraphic(level[curLevel].enemyPortal[i].rect);
	}
	for(var i = 0; i < level[curLevel].exitPortal.length; i++) {
		mainScene.removeGraphic(level[curLevel].exitPortal[i].rect);
	}
	for(var i = 0; i < level[curLevel].block.length; i++) {
		mainScene.removeGraphic(level[curLevel].block[i].rect);
	}
	
	for(var i = 0; i < level[l].label.length; i++) {
		mainScene.addGraphic(level[l].label[i].textObj);
	}
	for(var i = 0; i < level[l].enemy.length; i++) {
		mainScene.addGraphic(level[l].enemy[i].rect);
	}
	for(var i = 0; i < level[l].enemyPortal.length; i++) {
		mainScene.addGraphic(level[l].enemyPortal[i].rect);
	}
	for(var i = 0; i < level[l].exitPortal.length; i++) {
		mainScene.addGraphic(level[l].exitPortal[i].rect);
	}
	for(var i = 0; i < level[l].block.length; i++) {
		mainScene.addGraphic(level[l].block[i].rect);
	}
	
	player.controllable = (level[l].spawnX >= 0 || level[l].spawnY >= 0);
	healthbg.visible = health.visible = healthText.visible = player.controllable;
	mainScene.width = level[l].width;
	mainScene.height = level[l].height;
	player.rect.x = level[l].spawnX;
	player.rect.y = level[l].spawnY;
	player.xVel = 0;
	player.yVel = 0;
	mainScene.cameraSmooth = 0;
	updateCamera();
	mainScene.cameraSmooth = 5;
	curLevel = l;
}

installPaintRoutine(doAnimation);

function doAnimation(g) {
	if(!dAlpha) {
		g.opacity = 1;
		g.drawGraphic(0, 0, this.old);
	}
	g.fill = "black";
	g.opacity = this.oAlpha+=(dAlpha ? -0.04 : 0.04);
	g.fillRect(0, 0, width, height);
	if(this.oAlpha >= 1) {
		dAlpha = true;
	}
	if(this.oAlpha <= 0 && dAlpha) {
		uninstallPaintRoutine(doAnimation);
		this.old.deleteLater();
		delete oAlpha;
		this.paused = false;
	}
}

installMouseMoveListener(function(m) {
	mousePos = m.pos();
});

installMousePressListener(function(m) {
	var clickPos = m.pos();
	if(doesIntersect(clickPos.x, clickPos.y, startGame.x, startGame.y, 1, 1, startGame.width, startGame.height)) {
		spectatingEnt = player;
		loadLevel(1, true);
	} else if(doesIntersect(clickPos.x, clickPos.y, levelEditor.x, levelEditor.y, 1, 1, levelEditor.width, levelEditor.height))
		new Dialog("NyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\nNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\nNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\nNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\nNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan\nNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan").open();
	else if(doesIntersect(clickPos.x, clickPos.y, exitGame.x, exitGame.y, 1, 1, exitGame.width, exitGame.height))
		exit();
	delete clickPos;
});

function Player(color) {
	this.defaultColor = color;
	this.xVel = 0;
	this.yVel = 0;
	this.onGround = false;
	this.rect = new GraphicsRect(16, 28, color);
	this.health = 100;
	this.dead = false;
	this.canSpawn = false;
	this.inPortal = false;
	this.controllable = true;
	mainScene.addGraphic(this.rect);
	
	this.process = function() {
		if(!this.controllable)
			return;
		if(this.canSpawn) {
			this.rect.x = level[curLevel].spawnX;
			this.rect.y = level[curLevel].spawnY;
			this.xVel = 0;
			this.yVel = 0;
			this.onGround = false;
			this.rect.opacity = 1;
			this.health = 100;
			this.rect.color = new Color((this.health / 100) * this.defaultColor.red, (this.health / 100) * this.defaultColor.green, (this.health / 100) * this.defaultColor.blue, this.defaultColor.alpha);
			this.inPortal = false;
			this.dead = false;
			this.canSpawn = false;
			updateHealth();
		}
		if(!this.dead) {
			if(isKeyDown(Event.Key_A)) {
				if(this.xVel > -15) {	
					this.xVel += -2;
				}
			}
			
			if(isKeyDown(Event.Key_D)) {
				if(this.xVel < 15) {
					this.xVel += 2;
				}
			}
			
			if(isKeyDown(Event.Key_W) && this.onGround) {
				this.yVel = -16;
			}
		} else {
			if(this.rect.opacity > 0)
				this.rect.opacity -= 0.02;
			else {
				this.rect.opacity = 0;
				this.canSpawn = true;
			}
		}
		if(!this.inPortal)
			processPhysics(this, true, true, false);
	}
	
	this.takeHealth = function(amt) {
		this.health -= amt;
		if(this.health <= 0) {
			this.health = 0;
			this.dead = true;
		}
		this.rect.color = new Color((this.health / 100) * this.defaultColor.red, (this.health / 100) * this.defaultColor.green, (this.health / 100) * this.defaultColor.blue, this.defaultColor.alpha);
		updateHealth();
	}
}


function Level(lwidth, lheight, lspawnX, lspawnY) {
	this.width = lwidth;
	this.height = lheight;
	this.spawnX = lspawnX;
	this.spawnY = lspawnY;
	this.block = [];
	this.enemy = [];
	this.enemyPortal = [];
	this.exitPortal = [];
	this.label = [];
}

function Block(x, y, bwidth, bheight, color) {
	this.rect = new GraphicsRect(bwidth, bheight, color.toString());
	this.rect.x = x;
	this.rect.y = y;
}

function CrazyEnemy(x, y, xVel, yVel, collideEnemy) {
	this.dir = 1 + Math.round(Math.random() * 1);
	this.xVel = xVel;
	this.yVel = yVel;
	this.onGround = false;
	this.rect = new GraphicsRect(16, 20, new Color(100, 0, 100, 255).toString());
	this.rect.x = x;
	this.rect.y = y;
	
	this.process = function() {
		var dirn = processPhysics(this, true, collideEnemy, true);
		if(dirn > 0) {
			this.dir = dirn;
		}
		delete dirn;
		if(this.dir == 1) {
			if(this.xVel < 10) {
				this.xVel += 5;
			}
		} else if(this.dir == 2) {
			if(this.xVel > -10) {
				this.xVel -= 5;
			}
		}
		if(Math.round(Math.random() * 20) == 1) {
			this.dir = 1 + Math.round(Math.random() * 1);
		}
		if(this.onGround && Math.round(Math.random() * 20) == 1) {
			this.yVel -= 25;
		}
	}
}

function DumbEnemy(x, y, dwidth, dheight, collideEnemy) {
	this.dir = 2;
	this.xVel = 0;
	this.yVel = 0;
	this.onGround = false;
	this.rect = new GraphicsRect(dwidth, dheight, new Color(180, 0, 0, 255).toString());
	this.rect.x = x;
	this.rect.y = y;
	
	this.process = function() {
		var dirn = processPhysics(this, true, collideEnemy, true);
		if(dirn > 0) {
			this.dir = dirn;
		}
		delete dirn;
		if(this.dir == 1) {
			if(this.xVel < 10) {
				this.xVel += 2;
			}
		} else if(this.dir == 2) {
			if(this.xVel > -10) {
				this.xVel -= 2;
			}
		}
	}
}

function EnemyPortal(x, y) {
	this.rect = new GraphicsRect(25, 25, new Color(15, 0, 0, 150).toString());
	this.rect.x = x;
	this.rect.y = y;
	var made = 0;
	
	this.process = function() {
		if(made < 10) {
			if(Math.round(Math.random() * 50) == 1) {
				enemy[enemy.length] = new CrazyEnemy(this.rect.x + (Math.random() * this.rect.width), this.rect.y + (Math.random() * this.rect.height), Math.random() * 25 - Math.random() * 25, Math.random() * 25 - Math.random() * 25);
				made++;
			}
		}
	}
}

function ExitPortal(x, y, gotoLevel) {
	this.rect = new GraphicsRect(40, 40);
	this.rect.x = x;
	this.rect.y = y;
	this.timer = 0;
	this.gotoLevel = gotoLevel;
	
	this.process = function() {
		this.timer += 0.05;
		this.rect.color = new Color(Math.abs(Math.sin(this.timer * 2) * 255),  Math.abs(Math.cos(this.timer * 1.5) * 255), Math.abs(Math.cos(this.timer * 1.1) * 255), 150);
		if(player.rect.x >= this.rect.x && player.rect.x + player.rect.width <= this.rect.x + this.rect.width && player.rect.y >= this.rect.y && player.rect.y + player.rect.height <= this.rect.y + this.rect.height + player.rect.height) {
			player.inPortal = true;
			player.rect.x += ((this.rect.x - player.rect.x) + (this.rect.width / 2) - player.rect.scale / (player.rect.width / 2)) / 10;
			player.rect.y += ((this.rect.y - player.rect.y) + (this.rect.height / 2) - player.rect.scale / (player.rect.height / 2)) / 10;
			player.rect.scale-=0.1;
			if(player.rect.scale <= 0) {
				player.rect.opacity = 0;
				player.rect.scale = 1;
				loadLevel(this.gotoLevel, true);
			}
		}
	}
}

function Label(x, y, text, color) {
	this.textObj = new GraphicsText(text, x, y);
	this.textObj.margin = 10;
	this.textObj.radius = 10;
	this.textObj.brush = new Color(0, 0, 0, 100);
	this.textObj.color = color.toString();
}

function processPhysics(ent, doBlocks, doEn, doPly) {
	var rtn;
	var collideable2 = [];
	collideable2 = collideable2.concat((doBlocks ? level[curLevel].block : []), (doEn ? level[curLevel].enemy : []), (doPly ? player : []));
	var collideable = [];
	var i2 = 0;
	var nx = ent.rect.x + ent.xVel;
	var ny = ent.rect.y + ent.yVel;
	var nw = ent.rect.width * 2;
	var nh = ent.rect.height * 2;
	for(var i = 0; i < collideable2.length; i++) {
		if(collideable2[i] == ent)
			continue;
		if(nx + nw >= collideable2[i].rect.x && nx - (nw / 2) <= collideable2[i].rect.x + collideable2[i].rect.width && ny + nh >= collideable2[i].rect.y && ny - (nh / 2) <= collideable2[i].rect.y + collideable2[i].rect.height)
			collideable[i2++] = collideable2[i];
	}
	delete nx;
	delete ny;
	delete nw;
	delete nh;
	delete i2;
	delete collideable2;
	
	if(!ent.onGround)
		if(ent.yVel < ent.rect.height)
			ent.yVel += 1.5;
	
	if(Math.abs(ent.yVel) > 0)
		ent.onGround = false;
	
	if(ent.xVel > 0) {
		ent.xVel--;
		if(ent.xVel < 0)
			ent.xVel = 0;
	} else if(ent.xVel < 0) {
		ent.xVel++;
		if(ent.xVel > 0)
			ent.xVel = 0;
	}
	
	if(ent.yVel > 0) {
		ent.yVel--;
		if(ent.yVel < 0)
			ent.yVel = 0;
	} else if(ent.yVel < 0) {
		ent.yVel++;
		if(ent.yVel > 0)
			ent.yVel = 0;
	}
	
	if(ent.rect.x + ent.xVel < 0) {
		ent.rect.x = 0;
		ent.xVel = 0;
		rtn = 1;
		if(debug) collisions++;
	} else if(ent.rect.x + ent.rect.width + ent.xVel > mainScene.width) {
		ent.rect.x = mainScene.width - ent.rect.width;
		ent.xVel = 0;
		if(debug) collisions++;
	} else {
		if(ent.xVel != 0) {
			for(var i = 0; i < collideable.length; i++) {
				if(debug) collisionC++;
				if(ent.xVel > 0) {
					if(ent.rect.x + ent.rect.width + ent.xVel > collideable[i].rect.x && ent.rect.x + ent.xVel < collideable[i].rect.x && ent.rect.y + ent.rect.height > collideable[i].rect.y && ent.rect.y < collideable[i].rect.y + collideable[i].rect.height) {
						ent.rect.x = collideable[i].rect.x - ent.rect.width;
						if((collideable[i] == player && (ent instanceof DumbEnemy || ent instanceof CrazyEnemy)) || ((collideable[i] instanceof DumbEnemy || collideable[i] instanceof CrazyEnemy) && ent == player)) {
							player.xVel = 10 + Math.random() * 5;
							player.yVel = -(10 + Math.random() * 5);
							player.takeHealth(10);
						}
						ent.xVel = 0;
						rtn = (ent == player ? collideable[i] : 2);
						if(debug) collisions++;
					}
				} else if(ent.xVel < 0) {
					if(ent.rect.x + ent.rect.width + ent.xVel > collideable[i].rect.x + collideable[i].rect.width && ent.rect.x + ent.xVel < collideable[i].rect.x + collideable[i].rect.width && ent.rect.y + ent.rect.height > collideable[i].rect.y && ent.rect.y < collideable[i].rect.y + collideable[i].rect.height) {
						ent.rect.x = collideable[i].rect.x + collideable[i].rect.width;
						if((collideable[i] == player && (ent instanceof DumbEnemy || ent instanceof CrazyEnemy)) || ((collideable[i] instanceof DumbEnemy || collideable[i] instanceof CrazyEnemy) && ent == player)) {
							player.xVel = -(10 + Math.random() * 5);
							player.yVel = -(10 + Math.random() * 5);
							player.takeHealth(10);
						}
						ent.xVel = 0;
						rtn = (ent == player ? collideable[i] : 1);
						if(debug) collisions++;
					}
				}
			}
		}
	}
	
	if(ent.rect.y + ent.yVel < 0) {
		ent.rect.y = 0;
		ent.yVel = 0;
		if(debug) collisions++;
	} else if(ent.rect.y + ent.rect.height + ent.yVel > mainScene.height) {
		ent.rect.y = mainScene.height - ent.rect.height;
		ent.yVel = 0;
		ent.onGround = true;
		if(debug) collisions++;
	} else {
		if(ent.yVel != 0) {
			for(i = 0; i < collideable.length; i++) {
				if(debug) collisionC++;
				if(ent.yVel < 0) {
					if(ent.rect.y + ent.yVel + ent.rect.height > collideable[i].rect.y + collideable[i].rect.height && ent.rect.y + ent.yVel < collideable[i].rect.y + collideable[i].rect.height && ent.rect.x + ent.rect.width > collideable[i].rect.x && ent.rect.x < collideable[i].rect.x + collideable[i].rect.width) {
						ent.rect.y = collideable[i].rect.y + collideable[i].rect.height;
						if((collideable[i] == player && (ent instanceof DumbEnemy || ent instanceof CrazyEnemy)) || ((collideable[i] instanceof DumbEnemy || collideable[i] instanceof CrazyEnemy) && ent == player)) {
							player.xVel = (Math.round(Math.random() * 1) == 1 ? -(10 + Math.random() * 5) : 10 + Math.random() * 5);
							player.takeHealth(10);
						} else
							ent.yVel = 0;
						rtn = collideable[i];
						if(debug) collisions++;
					}
				} else if(ent.yVel > 0) {
					if(ent.rect.y + ent.yVel + ent.rect.height > collideable[i].rect.y && ent.rect.y + ent.yVel < collideable[i].rect.y && ent.rect.x + ent.rect.width > collideable[i].rect.x && ent.rect.x < collideable[i].rect.x + collideable[i].rect.width) {
						ent.rect.y = collideable[i].rect.y - ent.rect.height;
						if((collideable[i] == player && (ent instanceof DumbEnemy || ent instanceof CrazyEnemy)) || ((collideable[i] instanceof DumbEnemy || collideable[i] instanceof CrazyEnemy) && ent == player)) {
							player.yVel = -(10 + Math.random() * 5);
							player.xVel = (Math.round(Math.random() * 1) == 1 ? -(10 + Math.random() * 5) : 10 + Math.random() * 5);
							player.takeHealth(10);
						} else
							ent.yVel = 0;
						rtn = collideable[i];
						ent.onGround = true;
						if(debug) collisions++;
					}
				}
			}
		} else {
			ent.onGround = false;
		}
	}
	
	ent.rect.x += ent.xVel;
	ent.rect.y += ent.yVel;
	delete collideable;
	return rtn;
}

function doesIntersect(x1, y1, x2, y2, w1, h1, w2, h2) {
	return (x1 >= x2 && x1 + w1 <= x2 + w2 && y1 >= y2 && y1 + h1 <= y2 + h2);
}

function dumpMethods(o) {
	for(var m in o)
		print(m);
}
