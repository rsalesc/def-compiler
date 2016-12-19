s = "(  {  [  ]  }  )  ,  ;  =  +  -  *  /  <  >  <=  >=  ==  !=  &&  ||  !";

print ',\n'.join(map(lambda x: "\"\\\\"+x+"\"", s.split()))
