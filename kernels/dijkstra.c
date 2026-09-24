/* MiBench network/dijkstra — shortest path algorithm */
#include <stdint.h>

#define NUM_NODES 32
#define INF 99999

static int graph[NUM_NODES][NUM_NODES];
static int dist[NUM_NODES];
static int visited[NUM_NODES];

static void init_graph(void) {
    for (int i = 0; i < NUM_NODES; i++)
        for (int j = 0; j < NUM_NODES; j++)
            graph[i][j] = (i == j) ? 0 : INF;

    /* Add some edges (deterministic pattern) */
    for (int i = 0; i < NUM_NODES - 1; i++) {
        graph[i][i+1] = (i * 7 + 3) % 20 + 1;
        graph[i+1][i] = graph[i][i+1];
        if (i + 2 < NUM_NODES) {
            graph[i][i+2] = (i * 13 + 5) % 30 + 5;
            graph[i+2][i] = graph[i][i+2];
        }
        if (i + 4 < NUM_NODES) {
            graph[i][i+4] = (i * 17 + 11) % 50 + 10;
            graph[i+4][i] = graph[i][i+4];
        }
    }
}

static int min_distance(void) {
    int min = INF, min_idx = -1;
    for (int v = 0; v < NUM_NODES; v++) {
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_idx = v;
        }
    }
    return min_idx;
}

static void dijkstra(int src) {
    for (int i = 0; i < NUM_NODES; i++) {
        dist[i] = INF;
        visited[i] = 0;
    }
    dist[src] = 0;

    for (int count = 0; count < NUM_NODES - 1; count++) {
        int u = min_distance();
        if (u == -1) break;
        visited[u] = 1;

        for (int v = 0; v < NUM_NODES; v++) {
            if (!visited[v] && graph[u][v] != INF &&
                dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }
}

int main(void) {
    init_graph();
    int total = 0;
    for (int iter = 0; iter < 10; iter++) {
        for (int src = 0; src < NUM_NODES; src++) {
            dijkstra(src);
            total += dist[NUM_NODES - 1];
        }
    }
    return total;
}
