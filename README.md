Seam carving is a content-aware technique that resizes an image while preserving its most “interesting parts.”
“Interesting parts” are sections where the energy value is the greatest. This is revealed using the dual-gradient energy function, which shows where there are rapid color gradients–i.e. strong borders between objects.
“Seams” are formed by identifying uninterrupted vertical and horizontal changes of pixels wherein the sum of the pixels’ energy values is minimal.
Seams are removed one by one.
