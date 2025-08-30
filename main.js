// Links Web - Main Entry Point

window.onload = () => {
    // Get canvas and context
    const canvas = document.getElementById('main-canvas');
    if (!canvas) {
        console.error("Canvas element not found!");
        return;
    }

    // Initialize components
    const graphics = new Graphics(canvas);
    const simulation = new Hecken();

    // --- File I/O Handlers ---
    const loadFile = (content) => {
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

    const dxfParser = new DxfParser();
    const loadDxfFile = (part, content) => {
        logMessage(`Loading DXF for part: ${part}`);
        const geometry = dxfParser.parse(content);
        simulation.loadDxf(part, geometry);
    };

    const ui = new UI(simulation, graphics, { saveFile: saveFile });
    ui.setupFileHandlers(loadFile);
    ui.setupDxfFileHandlers(loadDxfFile);

    // --- Logging ---
    const logMessage = (message) => {
        ui.addLogMessage(message);
        console.log(message); // Also keep logging to console for debugging
    };

    simulation.setLogger(logMessage);

    logMessage("Links Web application started.");


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
