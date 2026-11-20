const apiUrl = "http://localhost:8080/api";
const wsUrl = "ws://localhost:8081/ws/search";

const indexForm = document.querySelector("#index-form");
const indexThreads = document.querySelector("#index-threads");
const indexResults = document.querySelector("#index-results");

const searchForm = document.querySelector("#search-form");
const searchQuery = document.querySelector("#search-query");
const searchBody = document.querySelector("#search-results tbody");

const pipeline = document.querySelector(".pipeline-section");

const healthButton = document.querySelector("#health-button");
const statsButton = document.querySelector("#stats-button");
const statsResults = document.querySelector("#stats-results");

async function api_request(path, options = {}) {
    const response = await fetch(apiUrl+path, options);

    if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);

    return await response.json();
}

function add_pipeline_message(text) {
    const message = document.createElement("p");
    message.textContent = text;
    pipeline.appendChild(message);
}

function show_results(results) {
    searchBody.replaceChildren();
    for (const result of results) {
        const row = document.createElement("tr");
        for (const value of [result.doc_id, result.score, result.snippet]) {
            const cell = document.createElement("td");
            cell.textContent = String(value);
            row.appendChild(cell);
        }
        searchBody.appendChild(row);
    }
}

indexForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    const threads = Number(indexThreads.value);
    if (!Number.isInteger(threads) || threads <= 0) {
        add_pipeline_message("Please enter a valid number of threads.");
        return;
    }
    const button = indexForm.querySelector("button");
    button.disabled = true;
    indexResults.textContent = "Indexing...";

    try {
        const data = await api_request("/index", { method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ threads })});

        if (data.error) throw new Error(data.error);

        indexResults.textContent = `Indexed ${data.documents_indexed} documents in ${data.time_ms} ms.`;
    } catch (error) {
        indexResults.textContent = `Indexing error: ${error.message}`;
    } finally {
        button.disabled = false;
    }
});

searchForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    const query = searchQuery.value.trim();
    if (!query) {
        add_pipeline_message("Please enter a search query.");
        return;
    }

    const button = searchForm.querySelector("button");
    button.disabled = true;
    searchBody.replaceChildren();
    pipeline.textContent = "Connecting...";

    const socket = new WebSocket(wsUrl);
    let completed = false;

    socket.onopen = () => {
        pipeline.textContent = "Expanding query...";
        socket.send(JSON.stringify({ query }));
    };

    socket.onmessage = (event) => {
        let message = JSON.parse(event.data);

        if (message.stage === "query_expansion") {
            add_pipeline_message(`Original query: ${message.data.original}; Expanded terms: ${message.data.expanded_terms.join(", ")}`);
            add_pipeline_message("Searching the index...");

        } else if (message.stage === "keyword_search") {
            show_results(message.data.results);
            add_pipeline_message(`Found ${message.data.results.length} results in ${message.data.time_ms} ms.`);

        } else if (message.stage === "done") {
            completed = true;
            add_pipeline_message("Search complete.");
            socket.close();

        } else if (message.stage === "error") {
            completed = true;
            add_pipeline_message(`Error: ${message.data.error}`);
            socket.close();
        }
    };

    socket.onerror = () => {add_pipeline_message("WebSocket connection error.");};

    socket.onclose = () => {
        button.disabled = false;
        if (!completed) add_pipeline_message("Connection closed before search completed.");
    };
});

healthButton.addEventListener("click", async () => {
    try {
        const data = await api_request("/health");
        statsResults.textContent = `Server: ${data.status}; Ollama: ${data.ollama ? "available" : "unavailable"}`;
    } catch (error) {
        statsResults.textContent = `Health check error: ${error.message}`;
    }
});

statsButton.addEventListener("click", async () => {
    try {
        const data = await api_request("/stats");
        statsResults.textContent = `Documents: ${data.documents}; index time: ${data.index_build_time_ms} ms; threads: ${data.threads_used}`;
    } catch (error) {
        statsResults.textContent = `Stats error: ${error.message}`;
    }
});