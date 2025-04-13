// Name: Paul Nguyen
// Date: 04/13/2025
// Course: CS 374 - Operating Systems
// Programming Assignment 1: Basic Formulas

#include <stdio.h>
#include <math.h>

int main(void)
{
    int n;
    float radius;
    float height_top;
    float height_bottom;
    float area;
    float bottom;
    float height;
    float top_surface_area;
    float bottom_surface_area;
    float lateral_surface_area;
    float surface_area;
    float total_surface_area = 0.0;
    float volume;
    float total_volume = 0.0;
    const float pi = 3.14159265359;

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
            scanf("%f", &radius);

            printf("What is the height of the top area of the spherical segment (ha)? \n");
            scanf("%f", &height_top);

            printf("What is the height of the bottom area of the spherical segment (hb)? \n");
            scanf("%f", &height_bottom);

            printf("Entered data: R = %.2f ha = %.2f hb = %.2f.\n",
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

        printf("Total Surface Area = %.2f Volume = %.2f.\n", surface_area, volume);

        total_surface_area += surface_area;
        total_volume += volume;
    }

    printf("Total average results:\n");
    printf("Average Surface Area = %.2f ", total_surface_area / n);
    printf("Average Volume = %.2f\n", total_volume / n);

    return 0;
}
