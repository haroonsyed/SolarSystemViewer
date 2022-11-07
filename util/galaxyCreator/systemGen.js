class GravBody {
  name;
  mass = 2e30;
  position;
  velocity;
  isParticle = true;
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
  intensity = 1e10;
}

class Header {
  SIUnitScaleFactor = 1e12; // 1 light year (1e16)
  UniverseScaleFactor = 1.0;
  ambientStrength = 1.0;
  specularStrength = 0.0;
  phongExponent = 0.0;
  CameraPosition;
}

let header = new Header();
header.CameraPosition = {
  x: 0.0,
  y: 0.0,
  z: header.SIUnitScaleFactor, // (width of milky way is 100k light years)
};

let numberOfStars = 50000;
let xRange = header.CameraPosition.z * 2;
let yRange = header.CameraPosition.z * 2;
let baseVelocity = 0.0; //1e10;
let outputFileName = "../../assets/scenes/galaxy.json";
generationType = "random";

let system = [];

// Unset default factors are name, position, velocity,
for (let i = 0; i < numberOfStars; i++) {
  let body = new GravBody();
  body.name = String(i);
  if (generationType == "random") {
    body.position = {
      x: Math.random() * xRange - xRange / 2.0,
      y: Math.random() * yRange - yRange / 2.0,
      z: 0.0,
    };
    body.velocity = {
      x: Math.random() * baseVelocity - baseVelocity / 2.0,
      y: Math.random() * baseVelocity - baseVelocity / 2.0,
      z: 0.0,
    };
  } else if (generationType == "circle") {
    // Determine the number of radi to make each time step
    let radiusOfGalaxy = 8e6;//1e16 * 1e4;
    const G = 6.67e-11;

    // Place the black hole that goes at the center
    if (i == 0) {
      body.position = { x: 0.0, y: 0.0, z: 0.0 };
      body.velocity = { x: 0.0, y: 0.0, z: 0.0 };
      body.mass = 2e30; // Saggitarius A*
    } else {
      // vOrbit = G * M / R
      let angle = i * ((2 * Math.PI) / numberOfStars);
      let pX = radiusOfGalaxy * Math.cos(angle);
      let pY = radiusOfGalaxy * Math.sin(angle);
      let vOrbit = Math.sqrt((G * system[0].mass) / radiusOfGalaxy);
      body.position = { x: pX, y: pY, z: 0.0 };
      body.position = {x: 152.1e9, y: 0.0, z: 0.0};
      body.velocity = {
        x: -vOrbit * Math.sin(angle),
        y: vOrbit * Math.cos(angle),
        z: 0.0,
      };
    }
  }
  system.push(body);
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
