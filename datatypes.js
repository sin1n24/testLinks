// Links Web - Data Structures

// Links Web - Data Structures

class Point {
    constructor(x = 0, y = 0, heel = true) {
        this.x = x;
        this.y = y;
        this.heel = heel; // Used to determine if a line segment should be drawn from the previous point
    }

    set(x, y, heel = true) {
        this.x = x;
        this.y = y;
        this.heel = heel;
    }

    clone() {
        return new Point(this.x, this.y, this.heel);
    }

    add(other) {
        return new Point(this.x + other.x, this.y + other.y);
    }

    subtract(other) {
        return new Point(this.x - other.x, this.y - other.y);
    }

    multiply(scalar) {
        return new Point(this.x * scalar, this.y * scalar);
    }

    divide(scalar) {
        if (scalar === 0) return new Point();
        return new Point(this.x / scalar, this.y / scalar);
    }

    equals(other) {
        return this.x === other.x && this.y === other.y;
    }

    dist(other) {
        return Math.hypot(this.x - other.x, this.y - other.y);
    }

    // Rotates this point around a center point by a given angle in radians
    rota(center, angle) {
        const d = this.subtract(center);
        const newX = center.x + d.x * Math.cos(angle) - d.y * Math.sin(angle);
        const newY = center.y + d.x * Math.sin(angle) + d.y * Math.cos(angle);
        return new Point(newX, newY, this.heel);
    }

    // From point::__dir
    // Calculates the angle in radians of the vector from 'sub' to this point.
    __dir(sub) {
        const dx = this.x - sub.x;
        const dy = this.y - sub.y;
        let dir = Math.atan(Math.abs(dy) / Math.abs(dx));

        if (dx > 0 && dy > 0) { // Quadrant 1
            // dir = dir;
        } else if (dx < 0 && dy > 0) { // Quadrant 2
            dir = Math.PI - dir;
        } else if (dx < 0 && dy < 0) { // Quadrant 3
            dir = -Math.PI + dir;
        } else if (dx > 0 && dy < 0) { // Quadrant 4
            dir = -dir;
        } else {
            dir = 0;
        }
        return dir;
    }

    // From point::offset
    offset(sub, shift) {
        const conrod = this.dist(sub);
        if (conrod === 0) return this.clone();

        const newX = this.x + (shift.y * (this.x - sub.x) + shift.x * (this.y - sub.y)) / conrod;
        const newY = this.y + (shift.y * (this.y - sub.y) - shift.x * (this.x - sub.x)) / conrod;
        return new Point(newX, newY, this.heel);
    }
}

function sincos(angle) {
    return new Point(Math.cos(angle), Math.sin(angle));
}

class Cyclo {
    constructor(pos = new Point(), r = 0) {
        this.pos = pos;
        this.r = r;
    }

    set(pos, r) {
        this.pos = pos;
        this.r = r;
    }
}
