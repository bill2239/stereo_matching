stereo matching 
==========
Simple CPU implementation from scratch of Block Matching using simple Sum of Square difference, rank transform and census transform as matching cost. Defualt use OpenMP optimization (save about 200ms per image), also have the option to use Visual c++ (for windows user) concurrency module to parallel the code (slower than openMP in my case)   
You can either compile the code to executable with gcc or visual studio with Opencv installed. This have been tested in Windows 10 and ubuntu 20.04    

left image:  
![image](im1.png)  
right image:  
![image](im2.png)  
disparity image with fixed windows ssd:  
![image](depth.png)  
disparity image with rank transform:  
![image](depth_rank.png)  
disparity image with census transform:  
![image](depth_census.png)  

Clearly, choosing rank transform or census transform as matching cost make disparity map less noisy, probably the nature of these transform make left and right images less sensitive to illumination changes. p.s. using census transform is slower than using rank transform  

---
## Reqirement 

    $OPENCV

**Compile**

    bash ./build 

**Usage**

    ./stereo_mathing -left=<left image> -right=<right image> -max-disparity=<disparity range> -window_size=<window size for block matching> -tranwin_size=<window size for transformation> -output=<output image file> -parallel=<if you want to run parallel version> -cost=<matching cost function ex: rank or census> -windows=<yes if you want to use concurrrency module>

**Examples**

    ./stereo_matching -left=im1.png -right=im2.png -max-disparity=50 -window_size=6 -tranwin_size=7 -output=depth_new.png -parallel=yes -cost=rank -windows=yes

If you find this repo useful to you please consider click the button below to donate and support my work!
[![Buy Me A Coffee](https://www.buymeacoffee.com/assets/img/custom_images/yellow_img.png)](https://buymeacoffee.com/bill2239)