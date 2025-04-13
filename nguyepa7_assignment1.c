#include <stdio.h>
#include <math.h>

int main(void)
{
    int n;
    double radius;
    double height_top;
    double height_bottom;
    double area;
    double bottom;
    double height;
    double top_surface_area;
    double bottom_surface_area;
    double lateral_surface_area;
    double surface_area;
    double total_surface_area = 0.0;
    double volume;
    double total_volume = 0.0;
    const double pi = 3.14159265359;

    do
    {
        printf("How many spherical segments you want to evaluate [2-10]? \n");
        scanf("%d", &n);
    } while (n < 2 || n > 10);

    for (int i = 1; i <= n; i++)
    {
        printf("Obtaining data for spherical segment number %d\n", i);
        do
        {
            printf("What is the radius of the sphere (R)? \n");
            scanf("%lf", &radius);

            printf("What is the height of the top area of the spherical segment (ha)? \n");
            scanf("%lf", &height_top);

            printf("What is the height of the bottom area of the spherical segment (hb)? \n");
            scanf("%lf", &height_bottom);

            printf("Entered data: R = %.2lf ha = %.2lf hb = %.2lf.\n",
                   radius, height_top, height_bottom);

            if (radius <= 0 ||
                height_top <= 0 ||
                height_bottom <= 0 ||
                height_top >= radius ||
                height_bottom >= radius ||
                height_top < height_bottom)
            {
                printf("Invalid Input.\n");
            }

        } while (radius <= 0 ||
                 height_top <= 0 ||
                 height_bottom <= 0 ||
                 height_top >= radius ||
                 height_bottom >= radius ||
                 height_top < height_bottom);

        area = sqrt((radius * radius) - (height_top * height_top));
        bottom = sqrt((radius * radius) - (height_bottom * height_bottom));
        height = height_top - height_bottom;

        top_surface_area = pi * (area * area);
        bottom_surface_area = pi * (bottom * bottom);
        lateral_surface_area = 2 * pi * radius * height;

        surface_area = top_surface_area + bottom_surface_area + lateral_surface_area;
        volume = (1.0 / 6.0) * pi * height * ((3 * (area * area)) + (3 * (bottom * bottom)) + (height * height));

        printf("Total Surface Area = %.2lf Volume = %.2lf.\n", surface_area, volume);

        total_surface_area += surface_area;
        total_volume += volume;
    }

    printf("Total average results:\n");
    printf("Average Surface Area = %.2f ", total_surface_area / n);
    printf("Average Volume = %.2f\n", total_volume / n);

    return 0;
}
