;  BLENDED-DROP-SHADOW-LOGO
;  draw the specified text over a blended background using current gimp fg
;   and bg colors.  The finished blend has a drop shadow underneath that blends
;   to the specified bg-color
;  if the blend colors are specified as high intensity, the sharp option
;   should be enabled or the logo will come out blurry

(define (scale size percent) (* size percent))

(define (script-fu-textured-logo text-pattern tile-type text size fontname bg-color blend-fg blend-bg)
  (let* ((img (car (gimp-image-new 256 256 RGB)))
	 (b-size (scale size 0.1))
	 (b-size-2 (scale size 0.05))
	 (f-size (scale size 0.075))
	 (ds-size (scale size 0.05))
	 (ts-size (- b-size-2 3))
	 (text-layer (car (gimp-text-fontname img -1 0 0 text b-size TRUE size PIXELS fontname)))
	 (width (car (gimp-drawable-width text-layer)))
	 (height (car (gimp-drawable-height text-layer)))
	 (blend-layer (car (gimp-layer-new img width height RGBA_IMAGE "Blend" 100 NORMAL)))
	 (shadow-layer (car (gimp-layer-new img width height RGBA_IMAGE "Shadow" 100 NORMAL)))
	 (text-shadow-layer (car (gimp-layer-new img width height RGBA_IMAGE "Text Shadow" 100 MULTIPLY)))
	 (tsl-layer-mask (car (gimp-layer-create-mask text-shadow-layer BLACK-MASK)))
	 (drop-shadow-layer (car (gimp-layer-new img width height RGBA_IMAGE "Drop Shadow" 100 MULTIPLY)))
	 (dsl-layer-mask (car (gimp-layer-create-mask drop-shadow-layer BLACK-MASK)))
	 (old-fg (car (gimp-palette-get-foreground)))
	 (old-bg (car (gimp-palette-get-background)))
	 (old-pattern (car (gimp-patterns-get-pattern))))    
    (gimp-image-undo-disable img)
    (gimp-image-resize img width height 0 0)
    (gimp-image-add-layer img shadow-layer 1)
    (gimp-image-add-layer img blend-layer 1)
    (gimp-image-add-layer img drop-shadow-layer 1)
    (gimp-image-add-layer img text-shadow-layer 0)
    (gimp-selection-all img)
    (gimp-patterns-set-pattern text-pattern)
    (gimp-layer-set-preserve-trans text-layer TRUE)
    (gimp-bucket-fill text-layer PATTERN-BUCKET-FILL NORMAL 100 0 FALSE 0 0)
    (gimp-selection-none img)
    (gimp-edit-clear text-shadow-layer)
    (gimp-edit-clear drop-shadow-layer)
    (gimp-palette-set-background bg-color)
    (gimp-drawable-fill shadow-layer BG-IMAGE-FILL)
    (gimp-rect-select img b-size-2 b-size-2 (- width b-size) (- height b-size) REPLACE TRUE b-size-2)
    (gimp-palette-set-background '(0 0 0))
    (gimp-edit-fill shadow-layer)
    (gimp-selection-layer-alpha text-layer)
    (gimp-image-add-layer-mask img text-shadow-layer tsl-layer-mask)
    (gimp-palette-set-background '(255 255 255))
    (gimp-edit-fill tsl-layer-mask)
    (gimp-selection-feather img f-size)
    (gimp-palette-set-background '(63 63 63))
    (gimp-edit-fill drop-shadow-layer)
    (gimp-palette-set-background '(0 0 0))
    (gimp-edit-fill text-shadow-layer)
    (gimp-palette-set-foreground '(255 255 255))
    (gimp-blend text-shadow-layer FG-BG-RGB NORMAL SHAPEBURST-ANGULAR 100 0 REPEAT-NONE FALSE 0 0 0 0 1 1)
    (gimp-selection-none img)
    (gimp-palette-set-foreground blend-fg)
    (gimp-palette-set-background blend-bg)
    (gimp-blend blend-layer FG-BG-RGB NORMAL LINEAR 100 0 REPEAT-NONE FALSE 0 0 0 0 width 0)
    (plug-in-mosaic 1 img blend-layer 12 1 1 0.7 135 0.2 TRUE FALSE tile-type 1 0)
    (gimp-layer-translate text-layer (- b-size-2) (- b-size-2))
    (gimp-layer-translate blend-layer (- b-size) (- b-size))
    (gimp-layer-translate text-shadow-layer (- ts-size) (- ts-size))
    (gimp-layer-translate drop-shadow-layer ds-size ds-size)
    (gimp-selection-layer-alpha blend-layer)
    (gimp-image-add-layer-mask img drop-shadow-layer dsl-layer-mask)
    (gimp-palette-set-background '(255 255 255))
    (gimp-edit-fill dsl-layer-mask)
    (gimp-image-remove-layer-mask img drop-shadow-layer APPLY)
    (gimp-selection-none img)
    (gimp-layer-set-name text-layer text)
    (gimp-patterns-set-pattern old-pattern)
    (gimp-palette-set-foreground old-fg)
    (gimp-palette-set-background old-bg)
    (gimp-image-undo-enable img)
    (gimp-display-new img)))

(script-fu-register "script-fu-textured-logo"
		    "<Toolbox>/Xtns/Script-Fu/Logos/Textured..."
		    "Creates textured logos with blended backgrounds, highlights, and shadows"
		    "Spencer Kimball"
		    "Spencer Kimball"
		    "1996"
		    ""
		    SF-PATTERN "Text Pattern" "Fibers"
		    SF-ADJUSTMENT "Mosaic Tile Type\n Squares 0, HEX 1, Oct 2" '(0 0 2 1 1 0 1)
		    SF-STRING "Text String" "The GIMP"
		    SF-ADJUSTMENT "Font Size (pixels)" '(200 1 1000 1 10 0 1)
		    SF-FONT "Font" "-*-cuneifontlight-*-r-*-*-24-*-*-*-p-*-*-*"
		    SF-COLOR "Background Color" '(255 255 255)
		    SF-COLOR "Starting Blend" '(32 106 0)
		    SF-COLOR "Ending Blend" '(0 0 106))
