
import { Sidebar } from "@/components/layout/Sidebar";
import { Header } from "@/components/layout/Header";
import { EmailConnectionCard } from "@/components/dashboard/EmailConnectionCard";
import { StatsCard } from "@/components/dashboard/StatsCard";
import { EmailList } from "@/components/email/EmailList";
import { EmailDetail } from "@/components/email/EmailDetail";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { CaseList } from "@/components/cases/CaseList";

const Index = () => {
  return (
    <div className="flex h-screen bg-gray-50">
      <Sidebar />
      
      <div className="flex-1 flex flex-col overflow-hidden">
        <Header />
        
        <main className="flex-1 overflow-auto p-6">
          <div className="max-w-7xl mx-auto space-y-6">
            <StatsCard />
            
            <Tabs defaultValue="inbox" className="w-full">
              <TabsList>
                <TabsTrigger value="inbox">Inbox</TabsTrigger>
                <TabsTrigger value="cases">Cases</TabsTrigger>
                <TabsTrigger value="settings">Settings</TabsTrigger>
              </TabsList>
              
              <TabsContent value="inbox" className="mt-6">
                <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
                  <div className="h-[700px]">
                    <EmailList />
                  </div>
                  <div className="h-[700px]">
                    <EmailDetail />
                  </div>
                </div>
              </TabsContent>
              
              <TabsContent value="cases" className="mt-6">
                <div className="h-[700px]">
                  <CaseList />
                </div>
              </TabsContent>
              
              <TabsContent value="settings" className="mt-6">
                <div className="max-w-xl">
                  <EmailConnectionCard />
                </div>
              </TabsContent>
            </Tabs>
          </div>
        </main>
      </div>
    </div>
  );
};

export default Index;
