/*****************/
/* Shading stuff */
/*****************/

#include "config.h"

#include <libgimp/gimp.h>

#include "lighting_main.h"
#include "lighting_image.h"
#include "lighting_shade.h"


static GimpVector3 *triangle_normals[2] = { NULL, NULL };
static GimpVector3 *vertex_normals[3]   = { NULL, NULL, NULL };
static gdouble     *heights[3] = { NULL, NULL, NULL };
static gdouble      xstep, ystep;
static guchar      *bumprow = NULL;

static gint pre_w = -1;
static gint pre_h = -1;

/*****************/
/* Phong shading */
/*****************/

static GimpRGB
phong_shade (GimpVector3 *position,
	     GimpVector3 *viewpoint,
	     GimpVector3 *normal,
	     GimpVector3 *lightposition,
	     GimpRGB      *diff_col,
	     GimpRGB      *spec_col,
	     LightType    light_type)
{
  GimpRGB       ambient_color, diffuse_color, specular_color;
  gdouble      nl, rv, dist;
  GimpVector3  l, nn, v, n;

  /* Compute ambient intensity */
  /* ========================= */

  n = *normal;
  ambient_color = *diff_col;
  gimp_rgb_multiply (&ambient_color, mapvals.material.ambient_int);

  /* Compute (N*L) term of Phong's equation */
  /* ====================================== */

  if (light_type == POINT_LIGHT)
    gimp_vector3_sub (&l, lightposition, position);
  else
    l = *lightposition;

  dist = gimp_vector3_length (&l);

  if (dist != 0.0)
    gimp_vector3_mul (&l, 1.0 / dist);

  nl = 2.0 * gimp_vector3_inner_product (&n, &l);

  if (nl >= 0.0)
    {
      /* Compute (R*V)^alpha term of Phong's equation */
      /* ============================================ */

      gimp_vector3_sub (&v, viewpoint, position);
      gimp_vector3_normalize (&v);

      gimp_vector3_mul (&n, nl);
      gimp_vector3_sub (&nn, &n, &l);
      rv = gimp_vector3_inner_product (&nn, &v);
      rv = pow (rv, mapvals.material.highlight);

      /* Compute diffuse and specular intensity contribution */
      /* =================================================== */

      diffuse_color = *diff_col;
      gimp_rgb_multiply (&diffuse_color, mapvals.material.diffuse_ref);
      gimp_rgb_multiply (&diffuse_color, nl);

      specular_color = *spec_col;
      gimp_rgb_multiply (&specular_color, mapvals.material.specular_ref);
      gimp_rgb_multiply (&specular_color, rv);

      gimp_rgb_add (&diffuse_color, &specular_color);
      gimp_rgb_multiply (&diffuse_color, mapvals.material.diffuse_int);
      gimp_rgb_clamp (&diffuse_color);

      gimp_rgb_add (&ambient_color, &diffuse_color);
    }

  gimp_rgb_clamp (&ambient_color);

  return ambient_color;
}

/*
static void
get_normal (gdouble      xf,
	    gdouble      yf,
	    GimpVector3 *normal)
{
  GimpVector3 v1,v2,n;
  gint numvecs=0,x,y,f;
  gdouble val,val1=-1.0,val2=-1.0,val3=-1.0,val4=-1.0, xstep,ystep;

  x=(gint)(xf+0.5);
  y=(gint)(yf+0.5);

  xstep=1.0/(gdouble)width;
  ystep=1.0/(gdouble)height;

  val=mapvals.bumpmax*get_map_value(&bump_region, xf,yf, &f)/255.0;
  if (check_bounds(x-1,y)) val1=mapvals.bumpmax*get_map_value(&bump_region, xf-1.0,yf, &f)/255.0 - val;
  if (check_bounds(x,y-1)) val2=mapvals.bumpmax*get_map_value(&bump_region, xf,yf-1.0, &f)/255.0 - val;
  if (check_bounds(x+1,y)) val3=mapvals.bumpmax*get_map_value(&bump_region, xf+1.0,yf, &f)/255.0 - val;
  if (check_bounds(x,y+1)) val4=mapvals.bumpmax*get_map_value(&bump_region, xf,yf+1.0, &f)/255.0 - val;

  gimp_vector3_set(normal, 0.0,0.0,0.0);

  if (val1!=-1.0 && val4!=-1.0)
    {
      v1.x=-xstep; v1.y=0.0; v1.z=val1;
      v2.x=0.0; v2.y=ystep; v2.z=val4;
      n=gimp_vector3_cross_product(&v1,&v2);
      gimp_vector3_normalize(&n);
      
      if (n.z<0.0)
        n.z=-n.z;
      
      gimp_vector3_add(normal,normal,&n);
      numvecs++;
    }

  if (val1!=-1.0 && val2!=-1.0)
    {
      v1.x=-xstep; v1.y=0.0;    v1.z=val1;
      v2.x=0.0;    v2.y=-ystep; v2.z=val2;
      n=gimp_vector3_cross_product(&v1,&v2);
      gimp_vector3_normalize(&n);
      
      if (n.z<0.0)
        n.z=-n.z;
      
      gimp_vector3_add(normal,normal,&n);
      numvecs++;
    }

  if (val2!=-1.0 && val3!=-1.0)
    {
      v1.x=0.0;   v1.y=-ystep; v1.z=val2;
      v2.x=xstep; v2.y=0.0;    v2.z=val3;
      n=gimp_vector3_cross_product(&v1,&v2);
      gimp_vector3_normalize(&n);
      
      if (n.z<0.0)
        n.z=-n.z;
      
      gimp_vector3_add(normal,normal,&n);
      numvecs++;
    }

  if (val3!=-1.0 && val4!=-1.0)
    {
      v1.x=xstep; v1.y=0.0;   v1.z=val3;
      v2.x=0.0;   v2.y=ystep; v2.z=val4;
      n=gimp_vector3_cross_product(&v1,&v2);
      gimp_vector3_normalize(&n);
      
      if (n.z<0.0)
        n.z=-n.z;
      
      gimp_vector3_add(normal,normal,&n);
      numvecs++;
    }

  gimp_vector3_mul(normal,1.0/(gdouble)numvecs);
  gimp_vector3_normalize(normal);
}
*/

void
precompute_init (gint w,
		 gint h)
{
  gint n;
  gint bpp=1;

  xstep = 1.0 / (gdouble) width;
  ystep = 1.0 / (gdouble) height;

  pre_w = w;
  pre_h = h;

  for (n = 0; n < 3; n++)
    {
      if (vertex_normals[n] != NULL)
        g_free (vertex_normals[n]);
      if (heights[n] != NULL)
        g_free (heights[n]);

      heights[n] = g_new (gdouble, w);
      vertex_normals[n] = g_new (GimpVector3, w);
    }

  for (n = 0; n < 2; n++)
    if (triangle_normals[n] != NULL)
      g_free (triangle_normals[n]);

  if (bumprow != NULL)
    {
      g_free (bumprow);
      bumprow = NULL;
    }
  if (mapvals.bumpmap_id != -1)
    {
      bpp = gimp_drawable_bpp(mapvals.bumpmap_id);
    } 
  
  bumprow = g_new (guchar, w * bpp);

  triangle_normals[0] = g_new (GimpVector3, (w << 1) + 2);
  triangle_normals[1] = g_new (GimpVector3, (w << 1) + 2);

  for (n = 0; n < (w << 1) + 1; n++)
    {
      gimp_vector3_set (&triangle_normals[0][n], 0.0, 0.0, 1.0);
      gimp_vector3_set (&triangle_normals[1][n], 0.0, 0.0, 1.0);
    }

  for (n = 0; n < w; n++)
    {
      gimp_vector3_set (&vertex_normals[0][n], 0.0, 0.0, 1.0);
      gimp_vector3_set (&vertex_normals[1][n], 0.0, 0.0, 1.0);
      gimp_vector3_set (&vertex_normals[2][n], 0.0, 0.0, 1.0);
      heights[0][n] = 0.0;
      heights[1][n] = 0.0;
      heights[2][n] = 0.0;
    }
}

/********************************************/
/* Compute triangle and then vertex normals */
/********************************************/

void
precompute_normals (gint x1,
		    gint x2,
		    gint y)
{
  GimpVector3 *tmpv, p1, p2, p3, normal;
  gdouble     *tmpd;
  gint         n, i, nv;
  guchar      *map = NULL;
  gint bpp = 1;
  guchar mapval;


  /* First, compute the heights */
  /* ========================== */

  tmpv                = triangle_normals[0];
  triangle_normals[0] = triangle_normals[1];
  triangle_normals[1] = tmpv;

  tmpv              = vertex_normals[0];
  vertex_normals[0] = vertex_normals[1];
  vertex_normals[1] = vertex_normals[2];
  vertex_normals[2] = tmpv;

  tmpd       = heights[0];
  heights[0] = heights[1];
  heights[1] = heights[2];
  heights[2] = tmpd;

  if (mapvals.bumpmap_id != -1)
    {  
      bpp = gimp_drawable_bpp(mapvals.bumpmap_id);
    }
  
  bpp = gimp_drawable_bpp(mapvals.bumpmap_id);

  gimp_pixel_rgn_get_row (&bump_region, bumprow, x1, y, x2 - x1);

  if (mapvals.bumpmaptype > 0)
    {
      switch (mapvals.bumpmaptype)
        {
          case 1:
            map = logmap;
            break;
          case 2:
            map = sinemap;
            break;
          default:
            map = spheremap;
            break;
        }

      for (n = 0; n < (x2 - x1); n++)
	{
	  if (bpp>1)
	    {
	      mapval = (guchar)((float)((bumprow[n * bpp] +bumprow[n * bpp +1] + bumprow[n * bpp + 2])/3.0 )) ;
            } 
	  else
	    {
	      mapval = bumprow[n * bpp];
	    }

	  heights[2][n] = (gdouble) mapvals.bumpmax * (gdouble) map[mapval] / 255.0;
	}
    }
  else
    {
      for (n = 0; n < (x2 - x1); n++)
	{
	  if (bpp>1)
	    {
	      mapval = (guchar)((float)((bumprow[n * bpp] +bumprow[n * bpp +1] + bumprow[n * bpp + 2])/3.0 )) ;
	    } 
	  else
	    {
	      mapval = bumprow[n * bpp];
	    }
	  heights[2][n] = (gdouble) mapvals.bumpmax * (gdouble) mapval / 255.0;
	}
    }
 
  /* Compute triangle normals */
  /* ======================== */

  i = 0;
  for (n = 0; n < (x2 - x1 - 1); n++)
    {
      p1.x = 0.0;
      p1.y = ystep; 
      p1.z = heights[2][n] - heights[1][n];

      p2.x = xstep;
      p2.y = ystep;
      p2.z = heights[2][n+1] - heights[1][n];

      p3.x = xstep;
      p3.y = 0.0;
      p3.z = heights[1][n+1] - heights[1][n];

      triangle_normals[1][i] = gimp_vector3_cross_product (&p2, &p1);
      triangle_normals[1][i+1] = gimp_vector3_cross_product (&p3, &p2);

      gimp_vector3_normalize (&triangle_normals[1][i]);
      gimp_vector3_normalize (&triangle_normals[1][i+1]);

      i += 2;
    }

  /* Compute vertex normals */
  /* ====================== */

  i = 0;
  gimp_vector3_set (&normal, 0.0, 0.0, 0.0);

  for (n = 0; n < (x2 - x1 - 1); n++)
    {
      nv = 0;

      if (n > 0)
        {
          if (y > 0)
            {
              gimp_vector3_add (&normal, &normal, &triangle_normals[0][i-1]);
              gimp_vector3_add (&normal, &normal, &triangle_normals[0][i-2]);
              nv += 2;
            }

          if (y < pre_h)
            {
              gimp_vector3_add (&normal, &normal, &triangle_normals[1][i-1]);
              nv++;
            }
        }

      if (n <pre_w)
        {
          if (y > 0)
            {
              gimp_vector3_add (&normal, &normal, &triangle_normals[0][i]);
              gimp_vector3_add (&normal, &normal, &triangle_normals[0][i+1]);
              nv += 2;
            }

          if (y < pre_h)
            {
              gimp_vector3_add (&normal, &normal, &triangle_normals[1][i]);
              gimp_vector3_add (&normal, &normal, &triangle_normals[1][i+1]);
              nv += 2;
            }
        }

      gimp_vector3_mul (&normal, 1.0 / (gdouble) nv);
      gimp_vector3_normalize (&normal);
      vertex_normals[1][n] = normal;

      i += 2;
    }
}

/***********************************************************************/
/* Compute the reflected ray given the normalized normal and ins. vec. */
/***********************************************************************/

static GimpVector3
compute_reflected_ray (GimpVector3 *normal,
		       GimpVector3 *view)
{
  GimpVector3 ref;
  gdouble     nl;

  nl = 2.0 * gimp_vector3_inner_product (normal, view);

  ref = *normal;

  gimp_vector3_mul (&ref, nl);
  gimp_vector3_sub (&ref, &ref, view);

  return ref;
}

/************************************************************************/
/* Given the NorthPole, Equator and a third vector (normal) compute     */
/* the conversion from spherical coordinates to image space coordinates */
/************************************************************************/

static void
sphere_to_image (GimpVector3 *normal,
		 gdouble     *u,
		 gdouble     *v)
{
  static gdouble     alpha, fac;
  static GimpVector3 cross_prod;
  static GimpVector3 firstaxis  = { 1.0, 0.0, 0.0 };
  static GimpVector3 secondaxis = { 0.0, 1.0, 0.0 };

  alpha = acos (-gimp_vector3_inner_product (&secondaxis, normal));

  *v = alpha / G_PI;

  if (*v==0.0 || *v==1.0)
    {
      *u = 0.0;
    }
  else
    {
      fac = gimp_vector3_inner_product (&firstaxis, normal) / sin (alpha);

      /* Make sure that we map to -1.0..1.0 (take care of rounding errors) */
      /* ================================================================= */

      if (fac>1.0)
        fac = 1.0;
      else if (fac<-1.0) 
        fac = -1.0;

      *u = acos (fac) / (2.0 * G_PI);

      cross_prod = gimp_vector3_cross_product (&secondaxis, &firstaxis);

      if (gimp_vector3_inner_product (&cross_prod, normal) < 0.0)
        *u = 1.0 - *u;
    }
}

/*********************************************************************/
/* These routines computes the color of the surface at a given point */
/*********************************************************************/

GimpRGB
get_ray_color (GimpVector3 *position)
{
  GimpRGB       color;
  gint         x, f;
  gdouble      xf, yf;
  GimpVector3  normal, *p;

  pos_to_float (position->x, position->y, &xf, &yf);

  x = RINT (xf);

  if (mapvals.transparent_background && heights[1][x] == 0)
    {
      gimp_rgb_set_alpha (&color, 0.0);
    }
  else
    {
      color = get_image_color (xf, yf, &f);

      if (mapvals.lightsource.type == POINT_LIGHT)
        p = &mapvals.lightsource.position;
      else
        p = &mapvals.lightsource.direction;

      if (mapvals.bump_mapped == FALSE || mapvals.bumpmap_id == -1)
	{
	  color = phong_shade (position,
			       &mapvals.viewpoint,
			       &mapvals.planenormal,
			       p,
			       &color,
			       &mapvals.lightsource.color,
			       mapvals.lightsource.type);
	}
      else
        {
          normal = vertex_normals[1][(gint) RINT (xf)];

          color = phong_shade (position,
			       &mapvals.viewpoint,
			       &normal,
			       p,
			       &color,
			       &mapvals.lightsource.color,
			       mapvals.lightsource.type);
        }
    }

  return color;
}

GimpRGB
get_ray_color_ref (GimpVector3 *position)
{
  GimpRGB      color, env_color;
  gint        x, f;
  gdouble     xf, yf;
  GimpVector3 normal, *p, v, r;

  pos_to_float (position->x, position->y, &xf, &yf);

  x = RINT (xf);

  if (mapvals.transparent_background && heights[1][x] == 0)
    {
      gimp_rgb_set_alpha (&color, 0.0);
    }
  else
    {
      color = get_image_color (xf, yf, &f);

      if (mapvals.lightsource.type == POINT_LIGHT)
        p = &mapvals.lightsource.position;
      else
        p = &mapvals.lightsource.direction;

      if (mapvals.bump_mapped == FALSE || mapvals.bumpmap_id == -1)
	{
	  color = phong_shade (position,
			       &mapvals.viewpoint,
			       &mapvals.planenormal,
			       p,
			       &color,
			       &mapvals.lightsource.color,
			       mapvals.lightsource.type);
	}
      else
        {
          normal = vertex_normals[1][(gint) RINT (xf)];

          gimp_vector3_sub (&v, &mapvals.viewpoint,position);
          gimp_vector3_normalize (&v);

          r = compute_reflected_ray (&normal, &v);

          /* Get color in the direction of r */
          /* =============================== */

          sphere_to_image (&r, &xf, &yf);
          env_color = peek_env_map (RINT (env_width * xf),
				    RINT (env_height * yf));

          color = phong_shade (position,
			       &mapvals.viewpoint,
			       &normal,
			       p,
			       &env_color,
			       &mapvals.lightsource.color,
			       mapvals.lightsource.type);
        }
    }

  return color;
}

GimpRGB
get_ray_color_no_bilinear (GimpVector3 *position)
{
  GimpRGB      color;
  gint        x;
  gdouble     xf, yf;
  GimpVector3 normal, *p;


  pos_to_float (position->x, position->y, &xf, &yf);

  x = RINT (xf);

  if (mapvals.transparent_background && heights[1][x] == 0)
    {
      gimp_rgb_set_alpha (&color, 0.0);
    }
  else
    {
      color = peek (x, RINT (yf));

      if (mapvals.lightsource.type == POINT_LIGHT)
        p = &mapvals.lightsource.position;
      else
        p = &mapvals.lightsource.direction;

      if (mapvals.bump_mapped == FALSE || mapvals.bumpmap_id == -1)
	{
	  color = phong_shade (position,
			       &mapvals.viewpoint,
			       &mapvals.planenormal,
			       p,
			       &color,
			       &mapvals.lightsource.color,
			       mapvals.lightsource.type);
	}
      else
        {
          normal = vertex_normals[1][x];

          color = phong_shade (position,
			       &mapvals.viewpoint,
			       &normal,
			       p,
			       &color,
			       &mapvals.lightsource.color,
			       mapvals.lightsource.type);
        }
    }

  return color;
}

GimpRGB
get_ray_color_no_bilinear_ref (GimpVector3 *position)
{
  GimpRGB      color, env_color;
  gint        x;
  gdouble     xf, yf;
  GimpVector3 normal, *p, v, r;

  pos_to_float (position->x, position->y, &xf, &yf);

  x = RINT (xf);

  if (mapvals.transparent_background && heights[1][x] == 0)
    {
      gimp_rgb_set_alpha (&color, 0.0);
    }
  else
    {
      color = peek (RINT (xf), RINT (yf));

      if (mapvals.lightsource.type == POINT_LIGHT)
        p = &mapvals.lightsource.position;
      else
        p = &mapvals.lightsource.direction;

      if (mapvals.bump_mapped == FALSE || mapvals.bumpmap_id == -1)
        {
          pos_to_float (position->x, position->y, &xf, &yf);

          color = peek (RINT (xf), RINT (yf));

          gimp_vector3_sub (&v, &mapvals.viewpoint, position);
          gimp_vector3_normalize (&v);

          r = compute_reflected_ray (&mapvals.planenormal, &v);

          /* Get color in the direction of r */
          /* =============================== */

          sphere_to_image (&r, &xf, &yf);
          env_color = peek_env_map (RINT (env_width * xf),
				    RINT (env_height * yf));

          color = phong_shade (position,
			       &mapvals.viewpoint,
			       &mapvals.planenormal,
			       p,
			       &env_color,
			       &mapvals.lightsource.color,
			       mapvals.lightsource.type);
        }
      else
        {
          normal = vertex_normals[1][(gint) RINT (xf)];

          pos_to_float (position->x, position->y, &xf, &yf);
          color = peek (RINT (xf), RINT (yf));

          gimp_vector3_sub (&v, &mapvals.viewpoint, position);
          gimp_vector3_normalize (&v);

          r = compute_reflected_ray (&normal, &v);

          /* Get color in the direction of r */
          /* =============================== */
    
          sphere_to_image (&r, &xf, &yf);
          env_color = peek_env_map (RINT (env_width * xf),
				    RINT (env_height * yf));

          color = phong_shade (position,
			       &mapvals.viewpoint,
			       &normal,
			       p,
			       &env_color,
			       &mapvals.lightsource.color,
			       mapvals.lightsource.type);
        }
    }

  return color;
}
