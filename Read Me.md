Student Help Buddy  

 

Your name(s) & email addresses  

------------------------
Ewaoluwa Ajani 

S00268840@atu.ie 
------------------------
Maksym Batrak 

S00273952@atu.ie 
------------------------
Ewa Bender 

S00273126@atu.ie 
------------------------
Jack Donaghy 

S00268126@atu.ie 
------------------------



Links to:   

Trello Page  

https://trello.com/invite/b/67a4cd86390df9d4f430ea15/ATTIc86e8a93084d9e1388b6c4981db6d29316510C2E/student-help-buddy 

GitHub Team Page  

max-comp/student-help-buddy 


 

Links to any data gathered or data analysis planned  

https://www.researchgate.net/publication/368301459_Student_Attention_and_Distraction_in_Community_College 

https://www.researchgate.net/publication/387697885_THE_IMPACT_OF_TIKTOK'S_FAST-PACED_CONTENT_ON_ATTENTION_SPAN_OF_STUDENTS 

https://www.researchgate.net/publication/388708713_The_role_of_self-regulated_learning_in_modelling_the_relationships_between_learning_approaches_FoMO_and_smartphone_addiction_among_university_students 

https://www.tandfonline.com/doi/full/10.1080/2331186X.2024.2425205#d1e181 

https://www.researchgate.net/publication/378436816_Teaching_Students_How_to_Study 

https://files.eric.ed.gov/fulltext/EJ1097402.pdf 

 

 

Problem to be solved 

The project aims to tackle poor time management among students,
which often leads to procrastination, missed deadlines, and stress.
The solution will help students manage their time effectively by providing tools
like timers and checklists that keep them on track. 

  

Summary of the Project solution 

The "Student Help Buddy" project offers a comprehensive time management tool that includes features
such as a student timer, a checklist, and toggle switches for student activities. This solution allows
students to visually and interactively manage their tasks, ensuring that they stay focused and productive. 




  
List of Project Requirements   

Student Timer: Interaction, screen, buzzer for notifications.  

Checklist: Interaction and screen for visualizing tasks.  

Student Toggle: LED, interaction, screen, and on/off switch to enable or disable tasks.  

Other Equipment: On/off switch for the overall system. 

 

 


 

Initial Design  

![StudentBuddy concept01](https://github.com/user-attachments/assets/6ea302d8-9d1e-49ea-9c73-642cccb9f1f2)

Testing approach – how did you plan your software and hardwaretesting as well as evidence of tests carried out. 

Testing software--
We sadly couldn't efficiently test the software due to only finding the correct LCD screen type on a virtual service called wokwi later on

Thankfully as this project continues we can get more adept at it and use all it's functionalities

Testing hardware--
Testing hardware was also inefficient due to only having access to the arduino yuns during class times

It was still very possible and during these times I was able to measure and 3D print the casing and assemble the parts during class to test it out 👍

Security measures--

Blynk Cloud has multiple layers of security, it means our network security was already
protected sufficiently on the network side.
Every user should have a valid email address.

A explanation from Blynk itself:
"Blynk offers a built-in verification process Each device has its own unique OAuth token and Product ID.
A combination of both these fields grants access to the device only for your organization "

We also created a rigid enough casing - the design is made easy to assembe and disassemble,
due to the limitation of only being able to access the arduino yúns code



Future improvements planned and potential next steps in developing the idea further 
The next step in Student Help Buddy project is to improve both software and hardware to make final product complete and working without problems.

Software improvements:
Software part still needs to be fully done, so all functions like timer, checklist and toggle switches will work correctly.
We plan to make test cases to check that every function is working like expected, so we can find and fix errors and problems if they happen.
Since we've been working with Wokwi for a while, we can use it for more efficient testing, so we can check code before we put it on real device.

Hardware improvements
Hardware setup must be finished, so all parts – buttons, LEDs, screen and potentiometer – are installed properly and do not move or break.
The case will also be improved to make sure everything fits tightly and stays in place, so no parts disconnect or stop working.
We will check that buttons and switches work many times without problems and do not break if used a lot.

Final testing and making sure everything works
At the end, we will do full testing of software and hardware together, so we can be sure that everything is working without errors.
If some problems appear during tests, we will fix them and improve the device.
The goal is to make Student Help Buddy a fully working and useful tool for students to manage time better.

Expanding IoT functionality
Since this project is IoT-based, we can connect it to a cloud service like Blynk, Firebase, or AWS IoT to store and analyze student activity.
Using an API, we can send data about study sessions, completed checklists, and toggle usage to a server for later review.
This data can be visualized in an app or website, so students can see their study progress over time( feature for the very distant future).
We could also use AI or data analysis to give students personalized study recommendations based on their habits(feature for the very distant future).
