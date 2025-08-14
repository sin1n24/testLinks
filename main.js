// Links Web - Main Entry Point

window.onload = () => {
    console.log("Links Web application started.");

    // Get canvas and context
    const canvas = document.getElementById('main-canvas');
    if (!canvas) {
        console.error("Canvas element not found!");
        return;
    }

    // Initialize components
    const graphics = new Graphics(canvas);
    const simulation = new Hecken();
    const ui = new UI(simulation, graphics);

    // --- File I/O Handlers ---
    const loadFile = (content) => {
        // Here you would check if it's a .links or .dxf file
        // For now, assume .links
        simulation.open(content);
    };

    const saveFile = () => {
        const content = simulation.save();
        const blob = new Blob([content], { type: 'text/plain;charset=utf-8' });
        const a = document.createElement('a');
        a.href = URL.createObjectURL(blob);
        a.download = 'scene.links';
        a.click();
        URL.revokeObjectURL(a.href);
    };

    ui.setupFileHandlers(loadFile, saveFile);

    const dxfParser = new DxfParser();
    ui.setupFileHandlers(loadFile, saveFile);

    const loadDxfFile = (part, content) => {
        console.log(`Loading DXF for part: ${part}`);
        const geometry = dxfParser.parse(content);
        simulation.loadDxf(part, geometry);
    };

    ui.setupDxfFileHandlers(loadDxfFile);

    // Main loop
    function gameLoop(timestamp) {
        graphics.updateMouseState(); // Reset one-frame mouse flags
        simulation.proceed(graphics);
        ui.update();

        requestAnimationFrame(gameLoop);
    }

    // Start the loop
    requestAnimationFrame(gameLoop);
};
