# Пошукова система

Система призначена для пошуку по ключовому слову у датасеті.



- багатопотокова побудова інвертованого індексу у хеш-таблиці;
- розширення запиту через Ollama і модель `llama3`;
- пошук у синхронному REST API та через WebSocket з виводом коротких фрагментів документів, відсортованих за релевантністю;
- наявність ендпоінтів для перевірки стану й статистики індексу;
- консольний та веб клієнти;
- Docker Compose для запуску веб-клієнта, сервера й Ollama.

## Архітектура

Основні компоненти:

- `src/main.cpp` — запуск HTTP-сервера на порту `8080` та WebSocket-сервера на порту `8081`;
- `src/ApiHandler.cpp` — REST-маршрути `/api/*`;
- `src/WebSocketSession.cpp` — потокова обробка пошукових запитів;
- `src/SearchSystem.cpp` — побудова індексу, пошук і формування snippet;
- `src/InvertedIndex.cpp` та `src/HashTable.cpp` — інвертований індекс;
- `src/ThreadPool.cpp` — черга завдань і пул робочих потоків;
- `src/Ollama.cpp` — інтеграція з Ollama API;
- `web/` — інтерфейс пошукової системи;
- `data/train/` — текстові документи датасету;
- `docs/` — UML-діаграми проєкту.


## Підготовка даних

Датасет не додано в репозиторій через його великий вміст, тому необхідно додати його самостійно.

Документи мають зберігатися як окремі текстові файли у папці `data/train/`.

Ім'я файлу без розширення є `doc_id`. Для індексації читаються лише файли `*.txt`.

Завантажити дані можна за [посиланням](https://console.cloud.google.com/storage/browser/deepmind-gutenberg;tab=objects?prefix=&forceOnObjectsSortingFiltering=false). Для завантаження необхідно попередньо встановити Google Cloud CLI ([інструкція](https://docs.cloud.google.com/sdk/docs/install-sdk)). Далі потрібно перейти в терміналі в папку `data/` клонованого репозиторію та запустити команду:
```powershell
gcloud storage cp --recursive gs://deepmind-gutenberg/train .
```
! Повноцінний датасет займає близько 11 Гб, тому доцільно видалити частину файлів або запускати команди, що завантажують лише частину файлів, наприклад:
*усі файли, що починаються на 1 (близько 1/10 частини):*

```powershell
New-Item -ItemType Directory -Force train
gcloud storage cp "gs://deepmind-gutenberg/train/1*.txt" .\train\
```
Для розробки було використано дані з `10.txt` до `10002.txt`.

## Запуск через Docker Compose

Docker Compose запускає пошуковий сервер, Ollama і статичний веб-сайт:

```powershell
docker compose up --build
```

Після готовності сервісів можна відкрити веб-інтерфейс: http://localhost:8000.

Перший запуск триває довше через завантаження моделі та індексу залежностей.

Для зупинки контейнерів:

```powershell
docker compose down
```

Для повного видалення збереженої моделі Ollama також видаліть volume:

```powershell
docker compose down -v
```


## Використання

### Вебінтерфейс

Відкрийте http://localhost:8000 у браузері:

1. Вкажіть кількість робочих потоків і натисніть `Index`.
2. Після завершення індексації введіть запит і натисніть `Search`.
3. Кнопки `Health` і `Stats` показують стан сервера та статистику індексу.

Етапи пошуку відображаються в розділі `Search Pipeline`.

### Консольний клієнт

Після запуску Docker Compose клієнт можна запустити в контейнері сервера.
Спочатку побудуйте індекс, а потім виконуйте пошук. Приклад запитів:
```
docker compose exec search-server ./search --health
docker compose exec search-server ./search --index 8
docker compose exec search-server ./search --stats
docker compose exec search-server ./search "stars and galaxies"
```

Параметр `--index` приймає додатне ціле число потоків. Пошуковий запит
передається одним аргументом, тому текст із пробілами потрібно брати в лапки.
Клієнт виводить HTTP-відповідь сервера.

## API

Базова адреса REST API — `http://localhost:8080`. Тіла POST-запитів і
відповіді мають формат JSON. Наведені значення у відповідях — приклади.

### `GET /api/health`

Запит не має тіла. Відповідь `200 OK`:

```json
{"status":"OK","ollama":true}
```

- `status` — рядок `OK`;
- `ollama` — логічне значення доступності Ollama.

### `POST /api/index`

Запит:

```json
{"threads":8}
```

`threads` — кількість потоків, ціле число; якщо поле відсутнє, використовується
`8`. Для побудови індексу передавайте додатне значення.

Відповідь `200 OK`:

```json
{"status":"OK","documents_indexed":10746,"time_ms":12500}
```

- `status` — рядок `OK`;
- `documents_indexed` — ціла кількість проіндексованих документів;
- `time_ms` — цілий час побудови індексу в мілісекундах.

Якщо документ не вдалося прочитати, сервер повертає `500 Internal Server Error`
з тілом `{"error":"Failed to read a document during indexing"}`.

### `GET /api/stats`

Запит не має тіла. Відповідь `200 OK`:

```json
{"documents":10746,"index_build_time_ms":12500,"threads_used":8}
```

Усі поля — цілі числа: `documents` — кількість документів,
`index_build_time_ms` — час індексації в мілісекундах, `threads_used` —
кількість потоків.

### `POST /api/search`

Запит:

```json
{"query":"stars and galaxies","mode":"keyword"}
```

`query` — обов'язковий рядок; `mode` — обов'язковий рядок `keyword`.
Відповідь `200 OK`:

```json
{"results":[{"doc_id":"123","score":0.75,"snippet":"stars and galaxies"}],"time_ms":420}
```

`results` — масив документів, відсортованих за спаданням `score`; він може бути
порожнім. `time_ms` — цілий час пошуку в мілісекундах, включно з розширенням
запиту. Кожен елемент `results` містить:

- `doc_id` — рядок, ім'я файлу без розширення;
- `score` — число, частка знайдених у документі пошукових термінів;
- `snippet` — рядок із фрагментом документа.

Для невалідного JSON або неправильних полів сервер повертає `400 Bad Request`,
для помилки під час пошуку — `500 Internal Server Error`. Тіло помилки має
формат `{"error":"опис помилки"}`, де `error` — рядок.

`OPTIONS` — допоміжний запит CORS. Браузер може надсилати його перед запитами до API. Вручну викликати його для роботи системи не потрібно.

### WebSocket `ws://localhost:8081/ws/search` (під капотом при запуску пошуку у веб-клієнті)

Клієнт надсилає текстове JSON-повідомлення:

```json
{"query":"stars and galaxies"}
```

`query` — обов'язковий рядок. При успішному пошуку сервер послідовно надсилає:

```json
{"stage":"query_expansion","data":{"original":"stars and galaxies","expanded_terms":["stars","galaxies"]}}
{"stage":"keyword_search","data":{"results":[{"doc_id":"123","score":0.75,"snippet":"stars and galaxies"}],"time_ms":420}}
{"stage":"done"}
```

- `stage` — рядок, назва етапу.
- Для `query_expansion` поле `data.original` — початковий рядок запиту, а
  `data.expanded_terms` — масив рядків, отриманих від Ollama.
- Для `keyword_search` поля `data.results` і `data.time_ms` мають ті самі типи
  й значення, що й у відповіді REST-пошуку.
- Повідомлення `done` не містить `data`.

У разі помилки сервер надсилає повідомлення такого формату:

```json
{"stage":"error","data":{"error":"No valid search terms"}}
```

`data.error` — рядок із поясненням. Помилка може виникнути, зокрема, через
неправильне повідомлення, порожній запит, недоступне розширення запиту або
неможливість створити фрагмент документа.

## Допоміжні тестувальні файли

- `course_work_parallel_computing` — основний HTTP/WebSocket-сервер;
- `try_index` — перевірка побудови індексу;
- `time_threads` — порівняння часу індексації при різній кількості потоків;
- `try_ollama` — перевірка підключення до Ollama;
- `locustfile.py` - навантажувальне тестування(http://localhost:8089)

Приклад запуску порівняння потоків на Visual Studio:

```powershell
cmake --build build --config Release --target time_threads
.\build\Release\time_threads.exe
```