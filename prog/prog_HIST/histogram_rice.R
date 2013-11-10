setwd('/home/yann/Coding/prog/prog_HIST/')
rice<-read.table("hist.csv")
plot(rice, type="h", col="dark red", log = "y", yaxt = "n",xlab="Surface water occurrence (%)", ylab="Rice area hit by surface water (ha)")
axis(2, c(1,10,100,1000,10000,100000)) # draw y axis with required labels

# rice_1_20<-read.table("NETH_histogram_rice_1_20")
# plot(rice_1_20, type="h", col="dark red", log = "y", yaxt = "n")
# axis(2, c(1,10,100,1000,10000,100000)) # draw y axis with required labels