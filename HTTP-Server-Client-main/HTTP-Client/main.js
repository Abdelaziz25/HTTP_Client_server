
var qoute = [" \" Be yourself; everyone else is already taken\"","--Oscar Wilde"," \"Two things are infinite: the universe and human stupidity; and I'm not sure about the universe.\" ","--Albert Einstein","\"So many books, so little time.\"","--Frank Zappa"]

var j=6;

function newqoute()
{
    var i=Math.floor((Math.random()*qoute.length));
if(i%2!=0 )
{
    i++;
    if(i==j)
    {
        i=i-2;
    }    
}

if(i>qoute.length-1)
{
    i=0
    if(i==j)
    {
        i=i+2;
    }   
}

if(i==j && j==0)
{
    i=i+2
}
if(i==j)
{
    i=i-2;
}   
    j=i;
        document.getElementById("body").innerHTML=qoute[i]
        i++
        document.getElementById("head").innerHTML=qoute[i]

}