#include <stdio.h>
#include <math.h>

int main(void)
{
    float pi = 3.14159265359;
    int n;
    int radius;
    int height_top;
    int height_bottom;
    int area;
    int bottom;
    int height;

    float top_surface_area;
    float bottom_surface_area;
    float lateral_surface_area;
    float total_surface_area;
    float volume;

    do
    {
        printf("How many spherical segments you want to evaluate [2-10]?");
        scanf("%d", &n);
    } while (n < 2 || n > 10);

    for (int i = 1; i < n; i++)
    {
        printf("Obtaining data for spherical segment %d", i);
        printf("What is the radius of the sphere (R)?");
        scanf("%d", &radius);
        printf("What is the height of the top spherical cap (h1)?");
        scanf("%d", &height_top);
        printf("What is the height of the top area of the spherical segment (ha)?");
        scanf("%d", &height_bottom);
        printf("Entered data: R = %d, h1 = %d, ha = %d\n", radius, height_top, height_bottom);

        do
        {
            area = sqrt((radius * radius) - (height_top * height_top));
            bottom = sqrt((radius * radius) - (height_bottom * height_bottom));
            height = height_top - height_bottom;
            top_surface_area = pi * (area * area);
            bottom_surface_area = pi * (bottom * bottom);
            lateral_surface_area = 2 * pi * radius * height;
            total_surface_area = top_surface_area + bottom_surface_area + lateral_surface_area;
            volume = (1 / 6) * pi * height * ((3 * (area * area) + 3 * (bottom * bottom) + (height * height)));
        } while (radius > 0 && height_top > 0 && height_bottom > 0 && height_top < radius && height_bottom < radius && height_top >= height_bottom);
    }

    printf("Total average results:\n");
    printf("Average Surface Area = %.2f", total_surface_area / n);
    printf("Average Volume = %.2f", volume / n);
    return 0;
}
