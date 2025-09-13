CFLAGS = -std=c++23 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

Prometheus: main.cpp
	g++ $(CFLAGS) -o ../build/Prometheus main.cpp $(LDFLAGS)

.PHONY: run clean

run: Prometheus
	../build/Prometheus

clean:
	rm -f ../build/Prometheus