# Introduction
To make `Sunshine` use our [`Cloudgame`](https://github.com/MobinYengejehi/Apollo) `Service API` we did the following: (Every github commit explained at the following paragraphs)

1. [Add cloudgame source files](#add-cloudgame-source-files-click-to-see-commit)
2. [Add Cloudgame Service HTTP Server](#add-cloudgame-service-http-server-click-to-see-commit)
3. [Add Cloudgame Remote Request System](#add-cloudgame-remote-request-system-click-to-see-commit)
4. [Add PerformAPIRequest function](#add-performapirequest-function-click-to-see-commit)
5. [Add Cloudgame HTTP Service Path Handlers](#add-cloudgame-http-service-path-handlers-click-to-see-commit)
6. [Add Cloudgame Service serverinfo, applist](#add-cloudgame-service-serverinfo-applist-click-to-see-commit)
7. [Add Cloudgame Service appasset](#add-cloudgame-service-appasset-click-to-see-commit)
8. [Add Cloudgame Service cancel](#add-cloudgame-service-cancel-click-to-see-commit)
9. [Add Cloudgame Service get_clipboard, set_clipboard](#add-cloudgame-service-get_clipboard-set_clipboard-click-to-see-commit)
10. [Add Cloudgame Service launch, resume](#add-cloudgame-service-launch-resume-click-to-see-commit)
11. [Add Cloudgame Validation Service](#add-cloudgame-validation-service-click-to-see-commit)
12. [Send Cloudgame Port With Sunshine Service](#send-cloudgame-port-with-sunshine-service-click-to-see-commit)

Extra Documention:

1. [How To Set Cloudgame Service API URL](#how-to-set-cloudgame-service-api-url)
2. [How To Build Apollo In Windows](https://github.com/MobinYengejehi/Apollo/blob/dev/cloudgame_docs/windows_build.md)

# Add cloudgame source files [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/8af9a35b6d4b7cd38ef109d5575d018426176db5)
At this commit the `cloudgame.h` and `cloudgame.cpp` files added to the project.

These files contain the service source code and manage all requests coming from `Cloudgame Clients`.

1. At [`cloudgame.h`](https://github.com/MobinYengejehi/Apollo/commit/8af9a35b6d4b7cd38ef109d5575d018426176db5#diff-f8e96ba83f85946e11363cb9a39b3afffcc8d0ee12e0b11f14be306edc17b1cbR17) the `Cloudgame Service` namespace has been declared.
2. After that we declared the `Initialize` function at [`cloudgame.cpp`](https://github.com/MobinYengejehi/Apollo/commit/8af9a35b6d4b7cd38ef109d5575d018426176db5#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR4).
3. Also we added these two files to [`common.cmake`](https://github.com/MobinYengejehi/Apollo/commit/8af9a35b6d4b7cd38ef109d5575d018426176db5#diff-cdb5957ed44bbccb73caa3df1e3b0fb61bd2b0a8c2eff462dc3d3140a38b713fR67) that let the compiler know it must build these source files too.

# Add Cloudgame Service HTTP Server [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/38d06d6612f1f00d50d830b802a469b11daf4243)
At this commit we defined `Cloudgame Service` HTTP server which communicates with `Cloudgame API`.

1. [Here](https://github.com/MobinYengejehi/Apollo/commit/38d06d6612f1f00d50d830b802a469b11daf4243#diff-44e84a2a1ae238f9f248578bd2703bac78d1e8545d48ae1667159d48f49fdb22R59) we defined `Cloudgame Service HTTP Server` port map.
2. After that we [initialized](https://github.com/MobinYengejehi/Apollo/commit/38d06d6612f1f00d50d830b802a469b11daf4243#diff-eb6a110416af60a972465b8fe0a2872668110ced01a9ff0403bc973fadf8d0bbR1493) and created new [thread](https://github.com/MobinYengejehi/Apollo/commit/38d06d6612f1f00d50d830b802a469b11daf4243#diff-eb6a110416af60a972465b8fe0a2872668110ced01a9ff0403bc973fadf8d0bbR1599) to process Our `HTTP` server.

# Add Cloudgame Remote Request System [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/6285ebd95c3c449a38f57e6bcc90f8ef8c74211e)
At this commit we declared and defined a `Remote Request` system for `Cloudgame Service`.

This System allows `Cloudgame` create filtered http requests to communicate with `Cloudgame API`.

# Add PerformAPIRequest function [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/a5adb5dc01ca437225676d2cec91b36cf0065ef0)
At this commit we defined a new function called [`PerformAPIRequest`](https://github.com/MobinYengejehi/Apollo/commit/a5adb5dc01ca437225676d2cec91b36cf0065ef0#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR165) which creates a http request and calls the `Cloudgame API`.

This function calls to API and also checks and handles the error came from the api.

# Add Cloudgame HTTP Service Path Handlers [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f)
At this commit we defined the following paths for [`Cloudgame Service HTTP Server`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR161):

1. [`^/serverinfo`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR257) (GET)
2. [`^/applist`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR264) (GET)
3. [`^/appasset`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR271) (GET)
4. [`^/launch`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR278) (GET)
5. [`^/resume`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR285) (GET)
7. [`^/cancel`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR292) (GET)
8. [`^/actions/clipboard`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR299) (GET)
9. [`^/actions/clipboard`](https://github.com/MobinYengejehi/Apollo/commit/c05b64c867ad40e21a541111f5d9a10303a3ec5f#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR306) (POST)

# Add Cloudgame Service serverinfo, applist [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/247502fc958f4a138d22b4fbcfd982dd369863bd)
At this commit we defined [`^/serverinfo`](https://github.com/MobinYengejehi/Apollo/commit/247502fc958f4a138d22b4fbcfd982dd369863bd#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR267) and [`^/applist`](https://github.com/MobinYengejehi/Apollo/commit/247502fc958f4a138d22b4fbcfd982dd369863bd#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR361) path handles.

# Add Cloudgame Service appasset [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/7ad336430db4fe5fdeecbc4cee7d5d7bad92c1e8)
At this commit we defined [`^/appasset`](https://github.com/MobinYengejehi/Apollo/commit/7ad336430db4fe5fdeecbc4cee7d5d7bad92c1e8#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR384) path handle.

# Add Cloudgame Service cancel [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/d930b80389868893d34940f37ca7199256694c51)
At this commit we defined [`^/cancel`](https://github.com/MobinYengejehi/Apollo/commit/d930b80389868893d34940f37ca7199256694c51#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR415) path handle.

# Add Cloudgame Service get_clipboard, set_clipboard [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/2f8e4604cfc5bcbf91e5d9bd3bc6f19f300da139)
At this commit we defined [`^/actions/clipboard (GET)`](https://github.com/MobinYengejehi/Apollo/commit/2f8e4604cfc5bcbf91e5d9bd3bc6f19f300da139#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR434) and [`^/actions/clipboard (POST)`](https://github.com/MobinYengejehi/Apollo/commit/2f8e4604cfc5bcbf91e5d9bd3bc6f19f300da139#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR450) path handles.

# Add Cloudgame Service launch, resume [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/bfe1c37508a2577638329b96037a069d1ad25d29)
At this commit we defined [`^/launch`](https://github.com/MobinYengejehi/Apollo/commit/bfe1c37508a2577638329b96037a069d1ad25d29#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR471) and [`^/resume`](https://github.com/MobinYengejehi/Apollo/commit/bfe1c37508a2577638329b96037a069d1ad25d29#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR546) path handles.

Also we defined a function called [`MakeLaunchSession`](https://github.com/MobinYengejehi/Apollo/commit/bfe1c37508a2577638329b96037a069d1ad25d29#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR261) which setups the launch session for controlling the system.

# Add Cloudgame Validation Service [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/633a16854fbe7e12277d70d2909e2273f54e0b0e)
At this commit we completed the definition of [`ValidateRequest`](https://github.com/MobinYengejehi/Apollo/commit/633a16854fbe7e12277d70d2909e2273f54e0b0e#diff-3412029a16fee944aff07fb527aebbc0d56a80bf1366cd80d9a8c0f562917b2dR276) function.

This function checks the `Cloudgame JWToken` to see if it is available and valid or not at every request.

Also we added the [`cloudgame_service_url`](https://github.com/MobinYengejehi/Apollo/commit/633a16854fbe7e12277d70d2909e2273f54e0b0e#diff-6b2f0a449fdefd8930e23ef0dcd752beec69242e1303d77653f047c5e0766385R1194) property for `sunshine.conf` file at this commit.

# Send Cloudgame Port With Sunshine Service [(Click To See Commit)](https://github.com/MobinYengejehi/Apollo/commit/e1c2e2133f756e2db1d45857494a0afc3c06e9e1)
At this commit we added `Cloudgame HTTP Server Port` to [`Sunshine HTTP Server`](https://github.com/MobinYengejehi/Apollo/commit/e1c2e2133f756e2db1d45857494a0afc3c06e9e1#diff-eb6a110416af60a972465b8fe0a2872668110ced01a9ff0403bc973fadf8d0bbR877) response.

# How To Set Cloudgame Service API URL
To set `Cloudgame Service API URL` you must open the `sunshine.conf` file and add following code to that:

```conf
cloudgame_service_url = http://your.domain:yourport/path
```

Example:
```conf
cloudgame_service_url = http://10.202.9.24:7000/v1
```

Now restart the application. The service will work with new URL.