class GravBody {
  name;
  radius = 800e6;
  mass = 2e30;
  position;
  velocity;
  tilt = 0.0;
  rotationPeriod = 100000.0;
  meshFilePath = "../assets/models/sphere.obj";
  vertexShaderPath = "../assets/shaders/default.vs";
  fragmentShaderPath = "../assets/shaders/default.fs";
  diffuseMap = "../assets/textures/sun.jpg";
}

class Light {
  position = {
    x: 0.0,
    y: 0.0,
    z: 0.0,
  };
  color = {
    red: 1.0,
    green: 1.0,
    blue: 1.0,
  };
  intensity = 1e2;
}

class Header {
  PhysicsDistanceFactor = 1e6;
  PhysicsMassFactor = 1e6;
  UniverseScaleFactor = 40.0;
  CameraPosition;
}

let header = new Header();
header.CameraPosition = {
  x: 0.0,
  y: 0.0,
  z: 3e11,
};

let numberOfStars = 100000;
let xRange = header.CameraPosition.z * Math.tan(Math.PI / 4); // 45 degree
let yRange = xRange / (16.0 / 9);
let baseVelocity = xRange / header.PhysicsDistanceFactor;
let outputFileName = "megaGalaxy.json";
generationType = "random";

let system = [];

// Unset default factors are name, position, velocity,
for (let i = 0; i < numberOfStars; i++) {
  if (generationType == "random") {
    let body = new GravBody();
    body.name = String(i);
    body.position = {
      x: Math.random() * xRange,
      y: Math.random() * yRange,
      z: 0.0,
    };
    body.velocity = {
      x: Math.random() * baseVelocity,
      y: Math.random() * baseVelocity,
      z: 0.0,
    };

    system.push(body);
  }
}

// Write to json file
let GravBodies = { GravBodies: system };
let lights = { Lights: [new Light()] };

let json = JSON.stringify({
  ...header,
  ...GravBodies,
  ...lights,
});

var fs = require("fs");
fs.writeFile(outputFileName, json, "utf8", (err) => {
  if (err) {
    console.log(err);
  } else {
    console.log("Finished!");
  }
});
