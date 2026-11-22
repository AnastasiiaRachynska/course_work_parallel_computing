from locust import HttpUser, task, between
from random import choice

class SearchUser(HttpUser):
    wait_time = between(0.5, 1.5)

    queries = ("apple", "banana", "cherry")

    @task
    def search(self):
        with self.client.post(
            "/api/search",
            json={"query": choice(self.queries), "mode":"keyword"},
            name = "POST /api/search",
            catch_response=True
        ) as response:
            if response.status_code != 200:
                response.failure(f"Failed with status code {response.status_code}")
                return
            try:
                response_json = response.json()
            except ValueError:
                response.failure("Response is not valid JSON")
                return
            
            if (
                not isinstance(response_json, dict)
                or not isinstance(response_json.get("results"), list)
                or not isinstance(response_json.get("time_ms"), (int, float))
            ):
                response.failure("Invalid search response")
                return

            data = response_json

            for result in data["results"]:
                if (
                    not isinstance(result, dict)
                    or not isinstance(result.get("doc_id"), str)
                    or not isinstance(result.get("score"), (int, float))
                    or not isinstance(result.get("snippet"), str)
                ):
                    response.failure("Invalid result item")
                    return
            response.success()


    @task
    def health(self):
        with self.client.get("/api/health", catch_response=True) as response:

            try:
                data = response.json()
            except ValueError:
                response.failure("Response is not valid JSON")
                return

            if (
                not isinstance(data, dict)
                or not isinstance(data.get("status"), str)
                or not isinstance(data.get("ollama"), bool)
            ):
                response.failure("Invalid health response")
                return

            response.success()

    @task
    def stats(self):
        with self.client.get("/api/stats", catch_response=True) as response:

            try:
                data = response.json()
            except ValueError:
                response.failure("Response is not valid JSON")
                return

            if (
                not isinstance(data, dict)
                or not isinstance(data.get("documents"), int)
                or not isinstance(data.get("index_build_time_ms"), (int, float))
                or not isinstance(data.get("threads_used"), int)
            ):
                response.failure("Invalid stats response")
                return

            response.success()