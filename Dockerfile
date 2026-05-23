FROM node:22-bookworm-slim AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY package.json ./
COPY main ./main
COPY web ./web

RUN cmake -S main -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build

FROM node:22-bookworm-slim AS runtime

WORKDIR /app

ENV NODE_ENV=production
ENV HOST=0.0.0.0
ENV PORT=4173

COPY --from=build /app/package.json ./package.json
COPY --from=build /app/web ./web
COPY --from=build /app/main ./main
COPY --from=build /app/build ./build

EXPOSE 4173

CMD ["node", "web/server.mjs"]
