#!/usr/bin/env python3

import matplotlib.pyplot as plt

import sys
import plot_util

instance_file = "../data/xrb14233.tsp"
tour_file = "output/merged_tour.txt"

coordinates = plot_util.read_point_file_path(instance_file)

plot_util.read_and_plot_tour(coordinates, tour_file, ":")
plot_util.read_and_plot_tour(coordinates, "../data/xrb14233.tour", "k:")

plt.show()
