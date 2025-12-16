+++
date = '2025-12-14'
draft = true
title = 'Framerate Independence - Part 1: Refresh Rate and VSync'
+++

{{< katex >}}

A well known-issue plaguing real-time graphics, especially video games; is the difficult task of ensuring framerate independence across a wide variety of devices and configurations. This is important for several reasons. It's needed for the application to function correctly with regards to physics, simulation, animation, rendering, etc. Just about everything depends on this and it's critical to get it right.

More generally, the problem to solve is how to be independent of any kind of rate. Wether that's the rendering rate, refresh rate, framerate, whatver. Refresh rate is going to be the main subject of this post, but future posts will examine physics framerate, simulation framerate, CPU/GPU throughput and latency, etc.

For now, we can define **framerate independence** as the ability of our real-time application to function exactly the same across different framerate, or at least as similar as reasonably possible.

In the old days, video games were made for specific hardware and were the sole application running on the system. A Pacman arcade machine for example only has the Pacman game running on it, with the display/monitor and computer integrated into one machine. On simple systems such as these, you knew ahead of time exactly what the hardware was and what the refresh rate of the display was, simplfying development by a lot. Basically, you could design and test it on one machine and if it worked properly then, it would pretty much work properly on every other machine since the hardware was identical.

Today, the situation is much more complex. You have to deal with different monitor refresh rates, different hardware, different drivers, different Operating Systems, etc. These issues compound and make it exceptionally difficult to properly run an interactive real-time application with correct simulation, graphics, and a smooth user experience.

This is, unfortunately, a very difficult problem to solve. Almost no perfect solution exists, and compromises must be made. What's worse, in some aspects it's impossible to solve properly considering that there are things outside the control of the application developer. On certain platforms it's just simply impossible to solve the problem correctly. At that point, you have to just use hacks or workarounds and give up on the idea of a perfect solution.

This will be part 1 in a multi-part series where I will continually update and share my findings. I'll revisit this topic over the years and improve my understanding and implementations - and make sure to provide relatively frequent updates.


## Refresh Rate and GPU Scanout
It's widely known that every monitor or display device operates on the notion of displaying a new "frame" at some frequency, which is known as the **refresh rate**. A typical Desktop monitor might have a refresh rate of 60Hz, meaning that 60 times per second, the monitor updates the image on screen. The monitor gets the image data from a region of memory called the **framebuffer**, which is where the frame to be presented is stored. The framebuffer is typically never controlled directly by the application, and is usually managed in conjuction by the OS/windowing system and the graphics driver (the GPU).

There is a period of time every refresh cycle where the monitor does not display anything called the **Vertical Blanking Interval**, or just VBLANK for short. The other period of time, where the monitor is actively displaying something, is aptly named the **Active Video** period, or just active video. These two periods make up the entire time of a refresh cycle of a given monitor. For example, a given 60Hz monitor refreshes once every ~16ms, and maybe about 2ms of that 16ms is the VBLANK period, the rest of the 14ms being active video.

However, it's incorrect to say that the monitor "gets" or "retrieves" the framebuffer data, quite the opposite. The monitor is essentially a slave device that just displays whatever data is sent to it. It's the graphics card or display adapter that is responsible for sending the framebuffer data to the monitor over HDMI/DisplayPort, or whatever media is in use.

The term for this is **GPU scanout**, when the GPU *serially* sends the framebuffer data pixel-by-pixel to the monitor device. GPU scanout operates independently of GPU rendering, which can be confusing. It's entirely possible for the contents of the framebuffer to change midway during the scanout, producing inconsistent results. And in fact, it's this exact scenario that causes screen-tearing!

Most people will tell you that screen-tearing is due to improper synchronization with the VBLANK period or refresh rate, and while that is true, it ignores the underlying mechanism. The issue actually stems from GPU scanout not being synchronized with the framebuffer contents. We'll talk about how that relates to the refresh rate and VBLANK periods later.

Most people sort of pretend that the scanout is performed by the monitor rather than the graphics card, which is an incorrect mental model, but can still be used to understand the issue to a decently satisfying degree.

Either way, you need to understand that if the GPU is scanning out a frame serially (pixel-by-pixel), and the framebuffer contents change during the scanout; the scanout will still continue scanning out the framebuffer, but with the new contents partway. It's like if you're a news reporter reading off the TV script that they give you, and someone just changes the script on the TV while you're in the middle of reading it. You still continue [reading off from the script even though someone changed it midway](https://youtu.be/iplfWUtKMzI?t=47), which is jarring to say the least.

Clearly, we somehow need a way to ensure that the script doesn't change midway through the news reporter reading off the TV. Or in our land, we need a way to ensure the framebuffer doesn't change while the GPU is scanning out the framebuffer contents.


## Screen-Tearing and Multi-Buffering
But what is screen-tearing exactly? Visually, it's when it looks like there is a rip across the screen. [This](https://en.wikipedia.org/wiki/Screen_tearing) wikipedia article has a good picture showing what it looks like.

In the picture, there appears to be a very sharp cut across the screen. It looks like the top half was shifted over relative to the bottom half (or vice-versa), and in the wikipedia example there's two tears! The result is very unpleasant in motion picture.

As I explained before, screen-tearing results because the framebuffer contents are changed sometime during the GPU scanout, causing the first half of the scanout to be the old framebuffer contents, and the second half of the scanout to be of the new framebuffer contents. So one scanout is supposed to correspond to one frame, but can be two or sometimes more if the framebuffer is changed more than once during the scanout period. This is exactly why there is a TEAR, because the tear represents the sharp difference between the different frames in one scanout.

Technically, screen tearing doesn't happen on a still image, even if the framebuffer contents change during the period of one scanout. But if one frame is different than the next, and if the difference is especially noticeable, then tearing will be observed.

(You may wonder what this has to do with refresh rate or the VBLANK period, and I'll get to that. For now, just forget about those things and focus only on scanout and framebuffer synchronization.)

So how would one solve this issue? Clearly, we need a way to synchronize scanout with updating the framebuffer. In particular, we want to only change the framebuffer contents when the GPU is NOT in the middle of a scanout. Logically then, we need a period provided where the GPU does NOT do a scanout, and during that short period, we are free to change the framebuffer contents.

This short period of time in between scanouts, specifically in between the end of the last scanout and before the beginning of the next scanout; must be long enough so that we can change the framebuffer contents in time, but also not too long otherwise scanout will be throttled. Typically, this period of time in between scanouts is very short, and so we only have a small amount of time to modify the framebuffer before the next scanout.

Because we have a limited period of time to change the framebuffer, we only have a short window to do all of our rendering to the framebuffer, which amounts to wasted time where we can't do any rendering work during scanout. We have to deliberately limit ourselves to only render during this short window between scanouts, which is sort of awful right?

This is of course not how it works. Since it's beyond silly to limit ourselves to render only in a short period of time between scanouts, it makes more sense to render to another buffer, an offscreen buffer, which we then swap with the actual framebuffer during the period between scanouts. The offscreen buffer is called the **back buffer**, and the framebuffer that the GPU uses to scanout is called the **front buffer**. This way, we can maximize the time we have to render AND also swap the buffers during the time between scanouts---provided that buffer swapping is a much shorter operation than rendering, which it is. We can take advantage of the much shorter operation of buffer swapping (which can be as low as a few microsceonds or even less) to schedule it to be exactly in the period between scanouts.

So we've naturally derived the reason for multiple buffering, specifically double buffering in this example. The idea is that we always maintain an offscreen buffer called the "back buffer" (possibly more than one offscreen buffer in the cases of triple-buffering or higher) that we are rendering to that the GPU doesn't scanout from. The other buffer is the visible buffer, called the "front buffer", that the scanout always reads from.

When the time is right (during the next period between scanouts), we swap the back buffer with the front buffer, making the back buffer the new front buffer. Since swapping is very fast, we can easily do this in the period between scanouts and avoid swapping the framebuffer during a scanout.

With this solution, we can take as long as we like to render to the backbuffer and not waste time purposely waiting to render in a very short window. Doing this successfully and consistently ensures that the framebuffer contents will never change midway through a scanout, thus eliminating screen-tearing. There's more to say on this, but for now keep this mental model in mind.

I mentioned that buffer swapping is an operation that is short enough to be able to execute within the short period of time between scanouts. We would simply have to copy the contents of the back buffer into the front buffer, which is a relatively quick operation. Even better (and this is how it usually works), the GPU stores a pointer to the framebuffers. In a double-buffered setup, whichever of the two framebuffers that is being pointed to is considered the front buffer, and the other one the back buffer. Therefore, buffer swapping is simply just changing a single pointer that makes the back buffer the front buffer, and front buffer the back buffer. This is literally just a pointer change which is an absurdly fast operation that can easily complete within the period between scanouts.

You can sometimes configure the behaviour of framebuffer swapping with Graphics APIs such as OpenGL, Vulkan, DirectX, etc. The OpenGL extension `GLX_OML_swap_method` (only available with Linux/X11) allows you to configure how the framebuffer is swapped in a double-buffered setup. The setting `GLX_SWAP_EXCHANGE_OML` will basically just switch the pointer so that the front buffer becomes the back buffer, and back buffer becomes the front buffer. There is also `GLX_SWAP_COPY_OML` which will copy the contents of the back buffer into the front buffer.


## Vertical Blank and Refresh Rate Re-visited
This hypothetical "short period" between scanouts that we have discussed up to this point is exactly the same as the VBLANK period. They are one and the same.

*Now* it should start to make sense how these things are related. And hopefully you can see where this is going.

To recap, we defined the VBLANK period as a short period of time during the refresh cycle where the monitor does not display anything. This also coincides with the time that GPU does not scanout anything. In fact, these two time periods are exactly synchronized due to precise hardware timers and communication between the monitor and GPU.

Through standards like EDID (Extended Display Identification Data), the GPU is alerted of the refresh rate of the monitor so that the GPU feeds the monitor with video feed (during the active video period) consistently and in synch with the refresh rate. In other words, it's basically impossible for the GPU to scanout video at the incorrect time.

Contrary to what seems like popular belief, screen-tearing is NOT caused by the GPU outputting image data at the wrong time, rather that the GPU is scanning out a framebuffer WHILE the framebuffer contents are changed. The GPU is ALWAYS scanning out video at the right times (during the active video period), so tearing is caused by the *framebuffer* changing at the wrong time.

Picture the GPU -> Monitor scanout as a tightly synchronized stream where the GPU begins scanout exactly when VBLANK ends (and thus active video begins), and ends when the active video period is over (right at the beginning of the next VBLANK period). This isn't exactly how it works, but the idea is generally that scanout does NOT happen during VBLANK. VBLANK is aptly named, as nothing happens or changes during that period of time.

The VBLANK period can be understood as a time where the monitor *provides* a purposeful blank period in the refresh cycle where you shouldn't send it any video feed. During this provided period, you can change your image (swap the buffer) rather than changing it during active video, as the latter will cause the video feed to change mid-transmission and cause visual artifacts (screen-tearing).

So that's why people often omit discussions of GPU scanout and almost imply that it's the monitor that fetches the data at a fixed cycle, leaving the VBLANK period as a grace period where we can swap our framebuffer and avoid tearing. Even though this model is incorrect, it is sufficient in MOST cases (not all) to understand what is going on.

It is precisely because GPU scanout is perfectly synchronized with the monitor refresh rate AND that it acts independently of rendering; that it appears as if it is the monitor that is serially reading data from the framebuffer at regular uninteruppted intervals independent of when you change the framebuffer contents.

If you're more interested in the low level functions of the GPU scanout behaviour in relation to the monitor, then you can check out [this source](https://www.extron.com/article/uedid) on the EDID protocol (Extended Display Identification Data), which is used to calibrate GPU scanout to be in synch with the refresh rate and active video periods (among other things).

For now, we're going to stop here and sit with this high-level model, and continue onwards to further understand the problem.


## VSync
So how exactly do we solve screen-tearing? Obviously we need to synchronize our buffer swaps with the VBLANK period and refresh rate, but how is that accomplished?

This is *exactly* what VSync accomplishes. VSync stands for "Vertical Sync", as in *Vertical Blanking Interval Synchronization*. The way VSync works is that when you have fully rendered a frame to the back buffer, it will then instruct your graphics card to hold off on swapping the back and front buffers until you reach the next VBLANK period, at which point it will then peform the buffer swap. Note that VSync is only possible with a multi-buffered setup, as it's the framebuffer swap that is being synchronized here, NOT rendering. This is a good thing though, as it allows us to control when we render asynchronously with respect to framebuffer swapping, but that also comes with some heavy responsibilites and consequences that we'll soon see.

So now we've derived the reason for having multi-buffering + VSync, as together, they solve the problem of screen-tearing.

Already though, you might see that there can be issues with this approach. For example, what would happen if the rendering time takes too long (and/or is started too late), so that the next VBLANK period is missed? Well you would then have to wait an entire refresh cycle for the next VBLANK period to do a buffer swap, effectively halving your framerate and doubling latency. This is what the default VSync option does, and if you're not careful, you'll throw away precious time idling the GPU and outputting at a much lower framerate.

With a 60Hz refresh rate, if your rendering time takes longer than ~16.6ms, but not greater than ~33.3ms, then you're going to be missing every other VBLANK period. This will reduce your effective framerate down to 30Hz, or 30 FPS, which is rather undesirable.

The reverse case is also a source of problems. What if your rendering time is faster than the refresh rate? You might end up getting ahead and start rendering frames too far in advance, maintaining throughput at the refresh rate, but increasing the *latency* by X amount of frames, where X is how many frames you are ahead of. Failing to account for this will cause potentially unlimited latency and make your game feel unresponsive and sluggish. Most drivers will typically have methods to prevent too many frames from being queued up before a buffer swap happens, but you shouldn't rely on this behaviour as it's driver-dependent. You really want to make sure you control your rendering so you don't render ahead and queue up too many frames.

One way to solve this is to simply block or wait until the framebuffer has been swapped (or until you know the image is presented on the screen, which as we'll see later, is not very simple). The downside is that you let the GPU idle while you're waiting for the next framebuffer swap. The upsides are that it is easy to program, saves more power, and has low latency.

Another way to solve this problem is utilizing triple buffering (when you have two back buffers), where you choose not to wait on a framebuffer swap and immediately start rendering the next frame to the other back buffer, before the framebuffer swap happens. If the rendering gets far ahead enough, frames are discarded. I'll have to do a seperate post on triple-buffering and other buffering methods alone just because it's a whole topic.

<!-- Briefly though, I don't think triple-buffering should be used in every scenario as it adds latency and discards frames every now and then. The motivation for using triple-buffering is to have as high as possible framerate with VSYNC, which periodically causes missed VBLANKS. It also comes with the cost of higher latency, almost always an entire frame behind. It also puts difficult requirements on your simulation to have to produce results for the renderer more often. -->

<!-- Triple-buffering makes sense when we're talking about AAA studios that are using fairly complex engine architectures with deeply pipelined stages and a mulithreaded job system; but many of these AAA studios also purposely incur latency for improved framerate. Having better graphics tends to sell more shelf units it seems. -->


### Missed VBLANKS, Framerate, and Adapative VSync
Because VSync limits your framebuffer swap to only happen on regular intervals of the VBLANK period, your framerate (as in how often you swap the framebuffer) is limited by the refresh rate and how long your render takes. We can model this relationship with simple equations.

First, let's go over some basic math. To calculate the refresh *period* (the time between refreshes), you use this simple inverse relationship:

$$
\text{refresh\_period} = \frac{1}{\text{ refresh\_rate }} \iff \text{ refresh\_rate } = \frac{1}{\text{refresh\_period}}
$$

The same is true of converting between framerate and frame period (or frametime):

$$
\text{frame\_period} = \frac{1}{\text{ frame\_rate }} \iff \text{ frame\_rate } = \frac{1}{\text{frame\_period}}
$$

We'll use the term frame period for consistency with refresh period, but frametime and frame period are interchangeable. Keep in mind we're using "framerate" as the frequency of buffer swaps here. Framerate can mean different things in different contexts, but here it's directly how often we swap the buffer.

If we want to determine what the effective framerate actually is (due to missed VBLANKS), then we need to know on average how many VBLANKS we miss. This can be calcuated by a ceiling on how much time it takes to render on average (or in the worst case) divided by the refresh period:

$$ \lceil \text{render\_time} \div \text{refresh\_period} \rceil $$

Putting everything together, we get this formula for our effective framerate:

$$
\text{frame\_rate} = \frac{\text{refresh\_rate}}{\lceil \text{render\_time} \div \text{refresh\_period} \rceil}
$$

With this simple model, we can start evaluting what framerates we can expect for different refresh rates. Here is a list of refresh rates with the possible framerates as render time increases, and so VBLANK misses increase.

- For 60Hz: 60, 30, 20, 15, 12
- For 75Hz: 75, 37.5, 25, 18.75, 15
- For 120Hz: 120, 60, 40, 30, 24
- For 144Hz: 144, 72, 48, 36, 28.8
- For 240Hz: 240, 120, 80, 60, 48

The varying framerates that we can target with VSync are not consistent across refresh rates. If you wanted to target 60FPS, the closest you could get on 144Hz monitor would be 72FPS which is about a 2.8ms difference compared to running at 60FPS!

In real-time simulation, 2.8ms is a MASSIVE difference, and it's very difficult to design your engine to robustly account for such major differences. Practically speaking, you need to know what the minimum FPS is that you can run on most platforms, but that's difficult to determine exactly. If you have that, you at least have a maximum bound on your target FPS and can go from there.

It can get ugly quickly though. If you can only run at most 67 FPS, then you still won't be able to make 72FPS on 144Hz without screen tearing. In that case, you either choose to just accept tearing, or you drop down to the next available FPS with VSync which is 48FPS! Even though you can run at 67FPS, you are forced to go at 48FPS which stinks. This has the unintuitive result of making your app run *better* on a 60Hz monitor than a 144Hz monitor, because it can achieve a higher framerate without tearing. Most people who buy a 144Hz monitor do so because they want higher framerates, not lower!

It's no suprise then, that on PC, many games expose several options for the player to tune because it's just impossible to predict the characterstics of every device you ship to. In practice, many gamers (especially with comptetive games) turn off VSync so they can hit a decent framerate.

At some point, a high-enough refresh rate minimizes the effect of screen tearing by virtue of just refreshing so fast. Eventually it becomes unnoticeable, but it's debatable as to where the cutoff is, many people online with 240Hz monitors still complain about screen-tearing. Whatever the case, we'll probably need to support VSync for the foreseeable future.

Then there's also Adaptive VSync, which up until this point we've been ignoring. Adaptive VSync is an interesting compromise to this problem where render times are just a bit longer than the refresh rate. The idea is that if you miss a VBLANK, Adaptive VSync will just swap the framebuffer in the middle of a scanout even though it will certainly cause screen tearing. These swaps are called "late swaps", and they're a compromise between having VSync and having higher framerates. 

The idea is that it would better to render a bit late and with tearing, rather than halve your framerate just to avoid tearing. For example, if a game that was normally 60FPS suddenly spiked a little above, and if adaptive VSync was turned on, then it would just swap as soon as it's ready rather than waiting for the next VBLANK.

What's tricky to determine is if you should use this for a situation where there's a mismatch between your application's framerate and the refresh rate. For example, if your application can most of the time run at around 70-75 FPS, but not consistently, AND you're on a 144Hz monitor; then you can enable adaptive Vsync and you will sort of achieve a stable 72FPS, but with occasional screen tears.

In a case like this where anywhere from 1%-10% of the time you can't render at 72FPS (maybe only at 50-60FPS), then adaptive vsync saves you but with screen tearing as the tradeoff. This could be a decent compromise, and Adaptive VSync is certainly a good solution that you can employ.

This is where triple-buffering with VSYNC comes in handy, as it allows you to start rendering right away instead of waiting for the next VBLANK. You'll get lower framerates still, but you won't have any tearing. The tradeoff is of course increased latency, but then you eliminate screen-tearing and get a better framerate on average.

The OpenGL `EXT_swap_control_tear` Is the extension that is available for Linux and Windows (GLX/WGL), that allows you to enable adaptive VSync by specifying that late swaps are allowed to happen.

<!-- - don't take it from me, test this on your own machine and work out the math yourself. Use a higher vblank interval on your device that will produce an uneven mutlitple, and measure the times and then do 1 divided by the time to get the actual framerate. -->


### VSync OFF Giving Better Latency
Many gamers turn off VSync for other reasons than just refresh-rate mismatching framerate. Most of the time, even when the game can run well within the refresh rate, gamers still prefer to switch off VSync. Why is this? The reason is very nuanced and most gamers (or even some game developers) don't understand what's really going on.

In theory, if you can consistently render within the refresh rate, then you can just turn on VSync and never miss a VBlank period (assuming that frames aren't buffered internally by the driver, which we'll talk about later). Your effective framerate is limited by the refresh rate, which is well understood even by the layman, so you should still get the highest framerate possible (exactly at the refresh rate). But gamers still turn off VSync to get better latency, how is this possible?

It's possible *because* of screen tearing. If you swap the framebuffer during scanout, then the scanout will start reading the new framebuffer data, and the screen will have old and new contents from the old and new rendered frames respectively.

This means that the portion of the screen update with the new rendered frame is technically more up to date than the old portion. Thus, you've basically took advantage of the serial update of the monitor (as fed by scanout) to unlock the refresh rate and effectively get sub-refresh-cycle latency.

The natural limitation of course is that the sub-refresh-cycle latencies are only for the portions of the screen that correspond to the point when the framebuffer was changed during scanout. If you do this multiple times per scanout, then you can even further reduce the sub-refresh-cycle latency, at the cost of inducing more tears, with each new section having less and less latency.

In practice, it manages to make the application feel more graphically responsive by locally reducing latency in certain portions of the screen. You can basically turn half of the screen of a 60Hz monitor into a 120Hz monitor with this method!. It doesn't work exactly like that, but I hope the idea is clear.

It's also true that for many first-person competitive games, that your gaze is typically focused towards the top of the screen rather than the bottom, because of 3D perspective making things father away look higher ([vanishing point](https://en.wikipedia.org/wiki/Vanishing_point)). It's also true that many monitors typically first update from the top and then go down line-by-line, technically making the top of your screen have better latency than the bottom, which at 60Hz, would be about a 14-16ms reduction in latency at the top of the screen!

So these are the reasons why it feels better to turn off VSync in competetive fast-paced games, because you're hacking the intended serial uninterrupted mechansim of monitor refreshes to achieve a higher framerate. Somehow, our eyes/brains can process the latency differential across the screen and synthesize it into a coherent image that appears to be updating faster.

Just to be clear, this is not an intended effect that any graphical application (that I know of) specifically aims for, but it is the natural side-effect of having a higher framerate than the refresh rate and not using VSync. It would be pretty cool if some games or applications targeted this as a *feature* in order to simulate higher refresh rates on monitors with lower refresh rates. The funny thing is, you would need to do the opposite of VSync and time your swaps to happen *during* the scanout period. You would actually want to try NOT to swap during VBLANKs, exactly the opposite of what VSync does. Currently, I don't know if that's even possible at all with current hardware and drivers, besides just trying to render at the highest framerate possible with no VSync.

The technical term for all of this is "sub-refresh-cycle latencies", and the folks over at blurbusters.com did an excellent review of this exact phenomenon in [this](https://blurbusters.com/understanding-display-scanout-lag-with-high-speed-video/) post. I would highly recommend checking it out, specifically the section titled "Sub-Refresh-Cycle Latencies Are Possible With VSYNC OFF". They did a much better job of explaining it over there.


## Caveats
All of this stuff we've been saying up to this point with VSync has been with many assumptions though, such as render time having an upper bound, VSync working properly, refresh rate being consistent, and being able to query the exact time a framebuffer swap happened. It may suprise you that usually NONE of these conditions are true, or at least, not all of them are true at the same time.

Having upper-bounded render time is a classic problem in real-time graphics, and is nevertheless still very difficult, especially as the demand for higher quality graphics remains. Even games released today such as the recent Death Stranding 2 that dropped this past summer, can only get 30FPS on the high quality mode on the PS5! There's also a performance mode on the game which gives a locked 60FPS, but it's 2025 and many people are starting to expect more than just 60FPS.

In practice, most developers have to choose a render time budget (such as ~16.6ms for 60FPS) and just stick with it. You have to make sure that the budget is never exceeded, or at least very rarely exceeded, and you also have to determine what your target hardware/platforms are. This is not an easy problem to solve, and that's why even on the PS5 games are still running at 30FPS or slower.

VSync itself is also rather unreliable. It's not consistently supported across driver implementations, and on PC, users usually have the option to force VSync off in their graphics card control panel. There is currently NO way to detect this at runtime. Vulkan, DirectX, and OpenGL do not provide any mechanisms to query if the user has forced VSync off. You just have to guess yourself using timers to see if VSync is behaving as expected.

Then there's also the refresh rate being inconsistent. It's not necessarily true that the refresh rate of a 60Hz monitor is exactly 60Hz, it might be 59.9Hz or something like that. This many seem trivial, but even minute differences can add up and you can quickly find yourself out of alignment in mere minutes or even seconds in some cases.

There's also VRR (Variable Refresh Rate) such as Nvidia G-Sync or AMD FreeSync. These technologies are very good solutions to the problems that we've discussed (especially screen-tearing), except that in practice, it's tricky to support them correctly. I'm not experienced at all with VRR, so don't take my word for it, but I'll investigate it in subsequent parts of this series.

Just to give you an idea though, the documentation for the Vulkan extension `VK_GOOGLE_display_timing` (which can be found [here](https://docs.vulkan.org/refpages/latest/refpages/source/VK_GOOGLE_display_timing.html)) makes note of VRR with this very insightful comment:

> This extension treats variable-refresh-rate (VRR) displays as if they are fixed-refresh-rate (FRR) displays.

As a graphics programmer I have no idea how to interpret what that means.

Lastly, and perhaps most importantly, it is not as widely supported as you might think to be able to know when a frame was presented. The aforementioned `VK_GOOGLE_display_timing` extension which gives you information on when framebuffer swaps happen, and when your framebuffer becomes visible; is not even supported on windows! It's mostly supported on Linux through the MESA drivers, and also on all Android platforms, but not on windows! That basically completely limits your windows port to treat vulkan as a sub-optimal option for a graphics backend. If you're running on windows you probably are using DirectX as your graphics backend anyways, so it's not too bad.

For OpenGL, we have the extension `OML_sync_control` which also gives us critical timing information for buffer swapping and VBLANK periods. And with DirectX, you can use the `DXGI_FRAME_STATISTICS` struct to get info on similar things.

I just want give huge thanks to Alan Ladavac and his seminal [post](https://medium.com/@alen.ladavac/the-elusive-frame-timing-168f899aec92) that highlighted the need for accurate presentation timing. As far as I understand, he was the one that pioneered bringing the `VK_GOOGLE_display_timing` to be more widely available on Linux/MESA which he presented in the GDC talk [here](https://www.gdcvault.com/play/1025031/Advanced-Graphics-Techniques-Tutorial-The/1000).


## In Practice - Example Synchronization
After all that boring theory and background it's time to finally look at some concrete implementations.

I'm going to be using OpenGL as the graphics API, but the same concepts apply generally. I'm also running my code on Linux, but you can do the same thing for other platforms as well. We also need VSync, which is provided by the `EXT_swap_control` extension. `EXT_swap_control_tear` provides adaptive VSync as well.

First, let's start off with a beginner render loop. This will be a mix of pseudo-code and actual code (note that this example uses double-buffering, but the code for that is omitted for brevity):

```c
time_t last_time = get_time();
time_t current_time = 0;
time_t frame_delta_time = 0;
glXSwapIntervalEXT(display, window, 1); /* provided by GLX_EXT_swap_control, this sets the swap interval to 1, meaning that we wait until the next VBLANK to swap, which is normal VSync. */
glClearColor(1.0f, 0.0f, 1.0f, 1.0f); /* sets the clear color to be a bright purple */
while(running) {
      simulation_result = simulate(frame_delta_time);

      glClear(GL_COLOR_BUFFER_BIT); /* clears the backbuffer to the purple clear color we set prior */
      render_to_framebuffer(simulation_result); /* renders to the backbuffer */

      glXSwapBuffers(); /* swaps the back and front buffers */

      current_time = get_time();
      frame_delta_time = current_time - last_time; /* getting how long this frame took relative to the last frame */
      last_time = current_time;
}
```

This is exactly what most tutorials will tell you to do---and it's totally wrong.

For one, the `SwapBuffers()` function is usually non-blocking, meaning that it is an asynchronous request that is sent out, and then the function returns immediately. This means that your frame time measurement following the `SwapBuffers()` function will be incorrect. All rendering commands are asynchronous in OpenGL (and other graphics APIs), meaning that by the time `SwapBuffers()` has completed, you may not even be finished rendering yet!

Also, If your application's simulation and render time is very short, then you will quickly queue up more frames running way ahead of the refresh rate. You need a mechanism to block your CPU from getting too far ahead of the GPU, or at least a mechanism to synchronize rendering so that the CPU can do other tasks while waiting for the next buffer swap before queuing up more rendering commands. (Or, with triple buffering, queue up rendering commands immediately after `SwapBuffers()`, and incur a frame of latency as a result).

Yet, if you try running this code, things will seem mostly okay. What's going on?

In fact, when I tested this on my machine connected to a 60Hz monitor, `frame_delta_time` was on average around 16.6ms. I was using a simple Hello Triangle example as a test, which should be running way ahead of the refresh rate, yet `frame_delta_time` was ~16.6ms on average---what gives?

The answer is that the graphics driver blocks you from rendering too many frames in advance. The driver has an internal mechanism for preventing too many frames from being queued up, and the way it accomplishes this is by blocking your render thread during the next GL render command that affects the framebuffer. In our case, the blocking happens during the `glClear()` command.

You can easily test this with code like this on your machine:

```c
time_t last_time = get_time();
time_t current_time = 0;
time_t frame_delta_time = 0;
glXSwapIntervalEXT(display, window, 1);
glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
while(running) {
      simulation_result = simulate(frame_delta_time);

      time_t time_before_clear = get_time();
      glClear(GL_COLOR_BUFFER_BIT);
      time_t time_after_clear = get_time();
      printf("glClear() time: %f\n", time_after_clear - time_before_clear);
      
      render_to_framebuffer(simulation_result);

      time_t time_before_swap = get_time();
      SwapBuffers();
      time_t time_after_swap = get_time();
      printf("SwapBuffers() time: %f\n", time_after_swap - time_before_swap);

      current_time = get_time();
      frame_delta_time = current_time - last_time;
      last_time = current_time;
}
```

With this code, you might be surprised to find that `glClear()` takes almost the entire time of a refresh period (~16.6ms on 60Hz). This is because the OpenGL driver is purposely blocking us here until the next framebuffer swap happens, making the buffer available for us to draw to. So in a very weird turn of events, it's not the function itself `SwapBuffers()` that is being synchronized to the framebuffer swap with VSync, but rather the next GL rendering command that affects the framebuffer.

And sure enough, this example code will give you very short times for `SwapBuffers()` (around 100 microseconds), confirming that it's non-blocking. This is because it is simply just sending a command out to the GPU/windowing system, without waiting for that command to complete (what it means for the command of buffer swapping to complete is itself an issue, as it doesn't necessarily correspond to the time that the frame becomes visible on screen).

It's better to think of `SwapBuffers()` as a command that just indicates to the GPU/windowing system that rendering work to the framebuffer (the back buffer) is finished. Then, when the next VBLANK comes, the buffers will actually be swapped.

One way to modify our code example to make it more correct would be to put the `glClear()` *right after* `SwapBuffers()`, so that we can effectively use it as a blocking mechanism to ensure that the framebuffer swap happened. Then our code for calculating `frame_delta_time` will be better.

Here is the simple change that we would make:

```c
time_t last_time = get_time();
time_t current_time = 0;
time_t frame_delta_time = 0;
glXSwapIntervalEXT(display, window, 1);
glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
while(running) {
      simulation_result = simulate(frame_delta_time);
      
      render_to_framebuffer(simulation_result);

      SwapBuffers();
      
      glClear(GL_COLOR_BUFFER_BIT); /* putting this right after SwapBuffers() blocks until the framebuffer is available again, sometime after the next VBLANK. */

      current_time = get_time();
      frame_delta_time = current_time - last_time;
      last_time = current_time;
}
```

This is still far from perfect.

With this method, `frame_delta_time` is on average about 16.6ms on my 60Hz setup which is what we would expect if we wanted correct behaviour per VSync. However, we can't just look at average, we also have to take into account the **variance**.

On my machine, I setup a simple test that measured the result of about 3000 frames (excluding the first 15 frames), and averaged the result. The average was 16.666ms (which is correct), but the highest time was 16.907ms and the lowest time was 16.418. These differences don't seem like much, but at worst, we have a difference of about 0.5ms from the highest to lowest times. Even the difference between the average and highest times is about 0.25ms - which is a lot of time in real-time graphics.

The reason why this is so bad is because the lost time of up to 0.25ms will cut into our render time budget for the next frame - EVEN if our rendering time is well under the refresh rate. If we render at 10ms and easily make the 16.6ms cutoff, but `glClear()` takes us 0.25ms over, we still lose that 0.25ms for the next frame and only have 16.35ms to render.

It's incorrect (as we've proven) to assume that `glClear()` finishes blocking the exact moment that our frame has been made visible on the screen. Even if it does, the blocking mechanism or OS thread scheduling may not wakeup our thread in time, adding some amount of latency which can vary. All of these things make it not very consistent, even though we are hitting VSync exactly on time!

Because the timing we get is not aligned with the time our framebuffer actually appears on screen, we'll be feeding incorrect values of `frame_delta_time` to our animation/simulation/physics with incorrect timing information, which throws everything off and causes stutter and other visual inconsistencies. It's this exact issue that Alan Ladavac talked about in his article which I mentioned before.

Keep in mind these simple timing tests were conducted in a very ideal environment as well. It's not always this good, these numbers are much better than what it would be for a real application that has to run for several hours at a time. I've had it be off by 1-2ms or even more sometimes. I've also been using a monitor with 60Hz refresh rate for my testing, but monitors with higher refresh rates will suffer even more! The period of a 240Hz monitor is ~4.17ms, and so even just 1ms is almost 25% of the refresh period! With 2ms, that's almost 50%!

Another major problem with this approach is that although `glClear()` blocks for the refresh period, it may not start doing so until mutliple frames have been queued up. What this means is that we could be one or more frames ahead before `glClear()` starts blocking. Ideally, we would hope that `glClear()` always blocks until the last framebuffer swap is executed, but there is nothing in any standards that explicitly mandates that behaviour.

In fact, I've found evidence that exactly the opposite happens. It seems that many drivers allow multiple "frames in flight", meaning that you could be 1 or more frames behind in latency. Check out [this](https://www.reddit.com/r/gamedev/comments/wdqbgv/why_do_modern_console_games_have_inherent_input), [this](https://stackoverflow.com/questions/41104667/how-many-frames-deep-is-a-modern-graphics-pipeline), and [this](https://community.khronos.org/t/flip-queue-aka-pre-rendered-frames/71364) link for more info.

Even ignoring what these people claim online, we still cannot *in principle* rely on `glClear()` to behave in line with VSync/double-buffering the way we expect it to. We have no reliable method of determining how many "frames in flight" the driver allows, as there's no visibility to the application for how many frames are queued up.


### Synchronization With glFinish()/glClear()
There is one other option we can consider, which is using `glFinish()` instead of `glClear()` for synchronization. This is a little bit better, as it seems to block right away instead of allowing frames to queue, but may not always be consistent. Still, `glFinish()` has timing issues and is not a good mechanism, but it might be better than `glClear()`.

One workaround for the variance in timing is smoothing out your delta time using some kind of interpolation. This is a fairly robust way to handle things in this situation so that you don't feed an incorrect frame delta time into your simluation/physics/etc that causes severe issues. Instead, you just smooth out any spikes so that it is mostly correct.

If you want to get more precise timing at the risk of sometimes being way off, then you can measure the time and make an educated guess as to if you've hit or missed the VBLANK. With a 60Hz refresh rate you would check to see if your time is close to 16.66ms, meaning you hit the VBLANK. If it's higher, then you try to guess exactly how many VBLANKS you missed.

If you think you didn't miss the VBLANK, then what you do is assume that your framebuffer is being swapped/presented exactly at the refresh rate. Calculate the refresh period (which you should do at program startup), and then just use that as your actual frame delta time! So on 60Hz you would just feed in 16.6ms regardless of what the measured time was. If you think you missed 1 VBLANK, then feed in 33.3ms, if you think you missed 2 VLBANKS, feed in 50ms, and so on for each of the possible refresh periods.

Here is example code that uses this method:

```c
time_t last_time = get_time();
time_t current_time = 0;
time_t frame_delta_time = 0;
float refresh_period = ((float)1 / refresh_rate) * 1000.0; /* in milliseconds */
float margin = 5.0; /* in milliseconds */
glXSwapIntervalEXT(display, window, 1);
glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
while(running) {
      float synthetic_frame_delta_time; /* in milliseconds */
      if(frame_delta_time &lt; refresh_period + margin) {
	    /* means we hit VBLANK */
	    synthetic_frame_delta_time = refresh_period;
      } else {
	    /* means we missed at least one VBLANK, possibly more */
	    int vblanks_missed = 1;
	    while(frame_delta_time &lt; refresh_period * (vblanks_missed + 1) + margin) {
		  ++vblanks_missed;
	    }
	    synthetic_frame_delta_time = refresh_period * vblanks_missed;
      }
      
      simulation_result = simulate(synthetic_frame_delta_time);

      glClear(GL_COLOR_BUFFER_BIT); /* putting this back here, where it won't block */
      render_to_framebuffer(simulation_result);

      SwapBuffers();
      glFinish(); /* this will block every frame until rendering is complete */
      
      current_time = get_time();
      frame_delta_time = current_time - last_time;
      last_time = current_time;
}
```

`margin` is set to be 5ms in this example, but in reality, you need to calibrate this value to be based on a combination of a percentage of the refresh period, and the margin of error that `glFinish()` gives at worst towards the higher end.

This code is an OK solution when we can't otherwise access presentation time, especially if your simulation + render time is well within the refresh period. A downside is you might still be getting multiple frames in flight depending on how `glFinish()` is implemented.

I could go on and on with solutions of how to workaround this complex issue using all kinds of crazy hacks and recalibration and timing tricks and all that, but it's not worth it. We could endlessly improve and refine this example, but there are far better methods using the timing extensions mentioned before. They're more complicated though, and since this post is getting too long I'm going to cut it off here.

In the next post, I'll start going over some REAL synchronization and timing solutions, using extensions that allow us to properly synchronize without resorting to compromises or unreliable hacks.


## Sources and Additional Research
- https://wikis.khronos.org/opengl/Swap_Interval
- link jon blow's QA framerate indepedencwe.
- https://developers.meta.com/horizon/blog/optimizing-vr-graphics-with-late-latching/
- https://www.activision.com/cdn/research/Hogge_Akimitsu_Controller_to_display.pdf
