# DistanceRegulation
This Program was made for learning purposes. The Program is simplified for a reason and is not an exact realtime simulation.

------------------------------------------------------------------------------------------------------------------------------

It is about 2 (or more) drivers that follows a path - n to n kilometers'. This path has its speed limits on specific waypoints and when the drivers hit these waypoints they must increase or decrease their speed.

You can define the acceleration (which is also the decceleration) speed for each driver.

------------------------------------------------------------------------------------------------------------------------------

The rules which define the program sequence are following:

Every driver but not the first one has to follow the frontman and must not overtake.

Every driver but not the first one has to keep a minimum distance to the frontman. The minimum distance is current speed of the driver in meter. i.e. the driver drives 50km/h the distance to the frontman has to be 50 meter. If the driver go below the minimum distance he needs to brake (deccelerate).

Every driver has to accelerate/deccelerate when they hit a waypoint with a new limit.

--------------------------------------------------------------------------------------------------------------------------------

All chunks of data are tracked for each driver every second of the program and gets written into a .csv file you can define in the defaults namespace. (pathToFile)

---------------------------------------------------------------------------------------------------------------------------------

I am open for any feedback and advice how to write code better in every form.
-------------------------------------------------------------
