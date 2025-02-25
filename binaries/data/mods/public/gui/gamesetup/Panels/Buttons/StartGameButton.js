class StartGameButton
{
	constructor(gamesetupPage, startGameControl, netMessages, readyControl, playerAssignmentsControl)
	{
		this.startGameControl = startGameControl;
		this.readyControl = readyControl;
		this.gameStarted = false;

		this.buttonHiddenChangeHandlers = new Set();

		this.startGameButton = Engine.GetGUIObjectByName("startGameButton");
		this.startGameButton.caption = this.Caption;
		this.startGameButton.onPress = this.onPress.bind(this);

		gamesetupPage.registerLoadHandler(this.onLoad.bind(this));
		playerAssignmentsControl.registerPlayerAssignmentsChangeHandler(this.update.bind(this));
	}

	registerButtonHiddenChangeHandler(handler)
	{
		this.buttonHiddenChangeHandlers.add(handler);
	}

	onLoad()
	{
		this.startGameButton.hidden = !g_IsController;
		for (let handler of this.buttonHiddenChangeHandlers)
			handler();
	}

	update()
	{
		let isEveryoneReady = this.isEveryoneReady();
		this.startGameButton.enabled = !this.gameStarted && isEveryoneReady;
		this.startGameButton.tooltip =
			!g_IsNetworked || isEveryoneReady ?
				this.ReadyTooltip :
				this.ReadyTooltipWaiting;
	}

	isEveryoneReady()
	{
		if (!g_IsNetworked)
			return true;

		for (let guid in g_PlayerAssignments)
			if (g_PlayerAssignments[guid].player != -1 &&
				g_PlayerAssignments[guid].status == this.readyControl.NotReady)
				return false;

		return true;
	}

	onPress()
	{
		if (this.gameStarted)
			return;

		this.gameStarted = true;
		this.update();
		this.startGameControl.launchGame();
	}
}

StartGameButton.prototype.Caption =
	translate("Start Game!");

StartGameButton.prototype.ReadyTooltip =
	translate("Start a new game with the current settings.");

StartGameButton.prototype.ReadyTooltipWaiting =
	translate("Start a new game with the current settings (disabled until all players are ready).");
