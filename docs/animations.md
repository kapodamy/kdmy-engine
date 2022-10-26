# Animations

>Revision 41

</br>
This document explain how animations can be done without hardcoding them in the engine source code.</br></br>
There two kind of animations:

- Macro based ([AnimationMacro](#animationmacro))
- Frame based ([Animation](#animation))
</br>

## XML example

---

```xml
<?xml version="1.0" encoding="utf-8"?>
<AnimationList atlasPath="default_atlas_for_all_anims.xml">
    <AnimationMacro name="my_macro" loop="1" frameRate="0" atlasPath="myatlas.xml" atlasPrefixEntryName="dance" atlasHasNumberSuffix="true">
        <Interpolator type="ease_in_out" property="x" start="50" end="75" duration="2000" />
        <Interpolator type="steps" property="x" start="50" end="75" stepsCount="50" stepsMethod="both" duration="500.7555" />
        <Set property="scaleX" value="2.0" />
        <Yield />
        <Pause duration="2000" />
        <Reset />
    </AnimationMacro>
    <Animation name="my_simple_animation" loop="-1" frameRate="24" atlasPath="my_atlas.xml">
        <FrameArray entryPrefixName="dancing" hasNumberSuffix="true" />
        <Frame entryName="dancing0015" />
        <Pause duration="24" />
    </Animation>
    <Animation name="alt_animation" loop="-1" frameRate="24" atlasPath="my_atlas.xml" alternatePerLoop="0">
        <FrameArray entryPrefixName="dancing left" hasNumberSuffix="true" />
        <AlternateSet />
        <FrameArray entryPrefixName="dancing right" hasNumberSuffix="true" />
        <AlternateSet />
        <FrameArray entryPrefixName="hey" hasNumberSuffix="true" />
    </Animation>
    <AnimationMacro name="random_transparency_animation" loop="-1">
        <!-- Pick a random value and change to alpha -->
        <RandomExact values="0 0.2 0.5 0.8 1" />
        <Set property="alpha" value="rnd" />
    </AnimationMacro>
    <AnimationMacro name="random_rotate_animation" loop="-1" atlasPath="my_atlas.xml">
        <RandomSetup start="90" end="240" />
        <RandomChoose />
        <Interpolator type="linear" property="rotate" start="0" end="rnd" duration="5000" />
        <Yield />
        <Interpolator type="linear" property="rotate" end="rnd" duration="2500" />
        <Yield />
        <Interpolator type="linear" property="rotate" end="0" duration="1000" />
        <RegisterSet register="1" value="123" />
        <RegisterProp register="0" property="translateX" />
        <RegisterSet register="2" value="rnd" />
        <Set property="translateY" value="reg1" />
        <Set property="translateX" value="reg2" />
    </AnimationMacro>
</AnimationList>
```

## AnimationList

The *AnimationList* element houses all declared animations. This element can have two optional [attributes](#animationlist-attributes) that indicates the default values for some animations.

---
</br>

### AnimationList Attributes

`atlasPath` Atlas filename or relative path to an atlas file. This will be the default atlas for those [frame based animations](#animation) that do not have the **atlasPath** attribute. (optional)

`frameRate` Default frame rate (fps) for [frame based animations](#animation). If this attribute is not present the default frame rate will be **24fps**. (optional)
</br>
</br>

### Notes
>
> #### *Frame animations must have an atlas*
>
> If the atlas file is not found or specified, the animation will be discarded. The `atlasPath` attribute must be present in all [Animation](#animation) elements if there not default atlas.
>
>
> #### *24 frames per second*
>
> **Adobe Animate** exports the spritesheet in 24fps, so this value is used for convenience.
>

---
---
</br>

## AnimationMacro

---

This is an advanced way to write animations, this done by manipulation the frames using **interpolators** and manually setting the sprite **properties**.
</br>
You can import various frames from an atlas by using `atlasPath`, `atlasPrefixEntryName`, and `atlasHasNumberSuffix` attributes.
</br>

### AnimationMacro Contents

- [AnimationMacro Attributes](#animationmacro-attributes)
- [Accesible properties](#accesible-properties)
- [Registers](#registers)
- [Instructions](#instructions)
  - [Interpolator](#interpolator-instruction)
  - [RandomSetup](#randomsetup-instruction)
  - [RandomChoose](#randomchoose-instruction)
  - [RandomExact](#randomexact-instruction)
  - [Set](#set-instruction)
  - [Yield](#yield-instruction)
  - [Pause](#pause-instruction)
  - [Reset](#reset-instruction)
  - [RegisterSet](#registerset-instruction)
  - [Registerprop](#registerprop-instruction)

---
</br>

## AnimationMacro Attributes

---

`name` The animation name, this must be unique in the animation list.
</br>

`loop` How many times the animation has to be repeated. Use `loop="0"` to repeat indefinitely or any
positive value to specify the amount of times.
</br>

`frameRate` Indicates whether the *duration* fields are expressed in frames. If you want define the fields in milliseconds set `frameRate` to zero.
</br>

`atlasPath` Path to the atlas file, must be relative to the current directory. In camera animation/manipulation this attribute should be not present.
</br>

`atlasPrefixEntryName`  Import all atlas entries which start with that name, this is case-sensitive. The atlas entries are imported as frames. You can choose which frame use by setting the `frameIndex` sprite property. Note: if you want use one frame, use the full entry name.
</br>

`atlasHasNumberSuffix`  Set to **true** to match only atlas entries which ends with numbers, otherwise, **false**. By default this value is **true**. (Optional)
</br>

`frameRestartIndex`  If an atlas is used, indicates which atlas frame should be applied when the animation is restarted and/or repeated (does not include loops). Use **-1** to not apply an atlas frame, otherwhise, the frame index (starts from zero). (Optional, defatuls to **-1**)</br>
Note: this changes the value of `frameIndex` property.
</br>

`frameAllowChangeSize` If an atlas is used, indicates whether the sprite draw size (`width` and `height` properties) should be changed when the property `frameIndex` is updated. Use **true** to update the draw size if `frameIndex` changes, otherwise, **false**. (Optional, defaults to **true**)
</br>

</br>

## Accesible properties

- `x` `y`: Relative coordinates of the top-left sprite corner, this is basically an offset. (Defaults to zero)
- `width` `height`: Draw size.
- `rotate`: Matrix rotation angle in radians, use with translation to specify the rotation pivot. (Defaults to zero)
- `scaleX` `scaleY`: Matrix scale factor. (Defaults to one)
- `skewX` `skewY`: Matrix skew factor. (Defaults to zero)
- `translateX` `translateY`: Matrix translation factor. (Defaults to zero)
- `alpha`: Sprite opacity/tranparency in a range [0.0, 1.0]. (Defaults to one)
- `z`: Z index, set to zero to use the asigned z index. (Defaults to zero)
- `frameIndex`: If an atlas was applied, this set what frame should use. (Defaults to zero whatever there an atlas or not)
- `vertexColorR` `vertexColorG` `vertexColorB`: vertex color RGB components in the range [0.0, 1.0]. (Defaults to zero)
- `offsetColorR` `offsetColorG` `offsetColorB` `offsetColorA`: vertex color RGBA components in the range [0.0, 1.0]. (Defaults to one)
- `pivotEnable`: (1)Enable or (0)disable the rotation from inside of the sprite. (Defaults to zero)
- `pivotU` `pivotV`: Rotation pivot coordinates. (Defaults to zero)
- `scaleDirectionX` `scaleDirectionY`: Defines the `scaleX` `scaleY` direction. Use *0* to scale from left/top to bottom/right, *-1* to scale in the inverse direction (from bottom/right to top/left) or *-0.5* to stretch the sprite keeping the center in the same position. (Defaults to zero)
- `translateRotation`  Indicates if should apply translate before rotate. (Defaults to zero)
- `scaleSize`  Indicates if only the draw size should be scaled. (Defaults to zero)
- `scaletranslation`. scales the `translateX` `translateY` values before scaling. (Defaults to zero)
- `flipX flipY` mirror the texture. (Defaults to zero)
- `flipCorrection` mirror the texture from the center, keeping the same draw location. (Defaults to one)
- `zoffset` offset to z index, avoids overwritting the z index value. (Defaults to zero)

</br>

---

### Registers

The animation macro offers 4 "registers" to store temporary values, these registers persists if the animation is looped.</br>You can access and read these values later using **reg0**, **reg1**, **reg2**, **reg3** as values.
</br>

---

## Instructions

---

Instructions defines how each frame should be animated by modifying the sprite properties. Of course you always animate one sprite.

</br>

### **Interpolator instruction**

Denotes a mathematical function that describes the rate at which a numerical value changes. More information: [CSS easing function](https://developer.mozilla.org/en-US/docs/Web/CSS/easing-function) and examples [here](https://developer.mozilla.org/en-US/docs/Web/CSS/easing-function#result).
</br></br>
`type` Easing algorithm. You can choose between linear, cubic Bézier presets, and staircase algorithms:

- **ease**  The interpolation starts slowly, accelerates sharply, and then slows gradually towards the end.
- **ease-in**  The interpolation starts slowly, and then progressively speeds up until the end, at which point it stops abruptly.
- **ease-out**  The interpolation starts abruptly, and then progressively slows down towards the end.
- **ease-in-out**  The interpolation starts slowly, speeds up, and then slows down towards the end.
- **steps** Defines a step function dividing the domain of output values in equidistant steps. This subclass of step functions are sometimes also called *staircase functions*.

`property` Sprite property to animate (see [Accesible properties](#accesible-properties)).
</br>

`start` Start value. (Optional)
</br>

`end` End value. (Optional)
</br>

`duration` The duration of the interpolation in frames or milliseconds. (See AnimationMacro [Attributes](#animation-attributes))
</br>

`stepsMethod` If the interpolator `type` is **steps**, this indicates wich function should be used (More info [CSS steps ease class](https://developer.mozilla.org/en-US/docs/Web/CSS/easing-function#the_steps_class_of_easing_functions)):

- **none** There is no jump on either end. Instead, holding at both the 0% mark and the 100% mark, each for 1/n of the duration. (Default value of `stepsMethod`)
- **start** Denotes a left-continuous function, so that the first step or jump happens when the interpolation begins.
- **end** Denotes a right-continuous function, so that the last step or jump happens when the interpolation ends.
- **both** Denotes a right and left continuous function, includes pauses at both the 0% and 100% marks, effectively adding a step during the interpolation iteration.

`stepsCount` If the interpolator `type` is **steps**, this indicates the number of steps.

---
</br>

### **RandomSetup instruction**

Setups a random number generator in the specified range.
</br>
</br>
Notes:

- Generated values can be `start`, `end` or any value between them.
- Generated values are decimal, in some fields like `frameIndex` the value will be truncated.
- **rnd** will hold the same value until [RandomChoose](#randomchoose-instruction) is executed.

</br>

`start` The start value. (Defaults to zero)

`end` The end value. (Defaults to one)

---
</br>

### **RandomChoose instruction**

Generates a new random value using the range defined by [RandomSetup](#randomsetup-instruction) instruction, this updates the value referenced by **rnd**.

---
</br>

### **RandomExact instruction**

Picks a random value on the `values` list, this updates the value referenced by **rnd**.

</br>

`values` a space-separated list of values, can be a mix of properties, registers, a numbers.
</br>Example: `<RandomExact values="123 -10 3.14 reg0 scaleX -180.550 rnd 0" />`
</br>
</br>
Notes:

- Any invalid value will be rejected, if the list is empty **rnd** will be zero.
- These values cannot be used to express **time**, if the selected value is used as duration (like in [Pause](#pause-instruction)), it will always be in **milliseconds**. The attribute `frameRate` of [AnimationMacro](#animationmacro-attributes) is ignored here.

---
</br>

### **Set instruction**

Change manually a sprite property.

</br>

`property` The sprite property to change.

`value` The value to set or **reg0**, **reg1**, **reg2**, **reg3** and **rnd** values. (Defaults to zero)

---
</br>

### **Yield instruction**

Stop the macro execution until all interpolators are completed.

</br>

`duration` Once all interpolators are completed do a [pause](#pause-instruction) for the specified amount of time. The value should be in frames or milliseconds, see AnimationMacro [Attributes](#animationmacro-attributes). (Optional)

---
</br>

### **Pause instruction**

Pauses the macro execution for the specified amount of time. The value should be in frames or milliseconds. (see AnimationMacro [Attributes](#animationmacro-attributes))
</br>

---
</br>

### **Reset instruction**

Sets all properties and registers to zero. With the following exceptions:

- The `width` and `height` will be have the same value as the frame N° 0 (`frameIndex`). Note: the size of the sprite is not modified if there are no appended frames (imported from an atlas, see AnimationMacro [Attributes](#animationmacro-attributes)).
- The `scaleX` and `scaleY` will be set to 1.

---
</br>

### **RegisterSet instruction**

Store a value in the desired register
</br>

`register`  Number of the register or his name, if this attibute is not present the value is applied to all registers. (Optional)

`value` Value to set. Can be a number, random value (**rnd**) or another register (**reg0**, **reg1**, **reg2**, **reg3**). (Optional, defaults to zero)

---
</br>

### **RegisterProp instruction**

Same as [RegisterSet](#yield-instruction), but picks a value from a sprite property.

</br>

`register` Number of the register or his name, if this attibute is not present the value applied to all registers. (Optional)

`property` Name of the sprite property to read the value.

---
</br>
</br>

## Animation macro tricks

---
</br>

1. In `width`/`height` sprite property set the value/start/end to **-1** to use the "frame size" (`frameWidth`/`frameHeight` from the atlas).
2. In all sprite properties use value **rnd** to assing a random value, you must setup the random range first using [RandomSetup](#randomsetup-instruction) instruction and generate a value using [RandomChoose](#randomchoose-instruction).
3. In all sprite properties use value **reg0**, **reg1**, **reg2** or **reg3** to use the values stored in these registers.
4. Use `pivotU`=0.5 and `pivotV`=0.5 to rotate the sprite from his center.
5. In [Yield](#yield-instruction) and [Pause](#pause-instruction) instructions is posible use **reg0**, **reg1**, **reg2**, **reg3** and **rnd** as well.
6. In the [Interpolator](#interpolator-instruction) instruction ignore `start` or `end` attibute to use the actual sprite property value as `start` or `end` value.
7. In [RegisterSet](#yield-instruction) and [RegisterProp](#yield-instruction) instructions omit the attribute `register` to set the value to all registers.
8. Use **rnd** in `register` attribute of [RegisterSet](#yield-instruction) and [RegisterProp](#yield-instruction) to choose a random register.

---
</br>

## Animation

---
This is the most simple way to make animations (by declaring frames).
</br>

### Animation Contents

- [Animation Attributes](#animation-attributes)
- [Imports](#imports)
  - [Frame](#frame)
  - [FrameArray](#framearray)
  - [Pause](#pause)
  - [AlternateSet](#alternateset)
</br>
</br>

## Animation Attributes

---

`name` The animation name, this must be unique in the animation list.
</br>

`loop` How many times the animation has to be repeated. Use `loop="0"` or `loop="-1"` to repeat indefinitely or any
positive value to specify the amount of times. If this attribute is not present the animation is looped indefinitely.
</br>

`frameRate` The animation frame rate (fps). If this value is zero, negative or is missing the **defualt frame rate** is used (see [AnimationList Attributes](#animationlist-attributes)).
</br>

`atlasPath` Atlas filename (or atlas relative path). If this attribute is not present the **default atlas** is used (see [AnimationList Attributes](#animationlist-attributes)).
</br>

`alternateInLoops` If [AlternateSet](#alternateset) is used, this indicates if a random set should be choseen on every animation loop. Set to **true** to use a random set on every loop, otherwise, **false** to use a random set when the animation is repeated. (optional, defaults to **false**)
</br>

</br>

## Imports

---
Indicates how the frames should be imported
</br>
</br>

### **Frame**

Imports a single frame from the atlas.
</br>

`entryName` The entry name in atlas. If this attribute is missing or empty, the name of the animation is used.

---
</br>

### **FrameArray**

Imports various frames from the atlas. Note: frames are not sorted, you need to have the frames sorted in the atlas first.
</br>

`entryPrefixName` Matches all atlas entries which start with that prefix, this is case-sensitive. If this attribute is missing or empty, the name of the animation is used. (optional)
</br>

`entrySuffixName` Matches all atlas entries which ends with a space following the suffix, this is case-sensitive. (optional)</br>
Example: using `entryPrefixName="dance"` and `entrySuffixName="alt"` will match "*dance alt*" and/or  "*dance alt0000*" entries.
</br>

`hasNumberSuffix` Set to **true** to match only atlas entries which ends with numbers, otherwhise, **false**. By default this value is **true**. Example: "dance*0000*", "dance*0001*", "dance*0002*", etc. (Optional)

---
</br>

### **Pause**

Repeats the last imported frame. Note: Do not put this at the start of the animation ok?.
</br>

`frames` How many times the last frame should be repeated, defaults to 1. (Optional)

---
</br>

### **AlternateSet**

Creates an alternative set of frames, declare frames between [AlternateSet](#alternateset) elements to create a set. A random set will be chosen each time the animation repeats.</br>
Note: putting an **AlternateSet** at the start or end of the animation will have no effect.

---
</br>
